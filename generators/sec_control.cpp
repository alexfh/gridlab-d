#include "sec_control.h"

CLASS *sec_control::oclass = NULL;
sec_control *sec_control::defaults = NULL;

static PASSCONFIG clockpass = PC_BOTTOMUP;

/* Class registration is only called once to register the class with the core */
sec_control::sec_control(MODULE *module)
{
	if (oclass == NULL)
	{
		oclass = gl_register_class(module, "sec_control", sizeof(sec_control), PC_PRETOPDOWN | PC_BOTTOMUP | PC_POSTTOPDOWN | PC_AUTOLOCK);
		if (oclass == NULL)
			throw "unable to register class sec_control";
		else
			oclass->trl = TRL_PROOF;

		if (gl_publish_variable(oclass,
			//**************** Sample/test published property - remove ****************//
			PT_complex, "test_pub_prop[kW]", PADDR(test_published_property), PT_DESCRIPTION, "test published property - has units of kW",
			//PID controller input
			PT_double, "f0[Hz]", PADDR(f0), PT_DESCRIPTION, "Nominal frequency in Hz",
			PT_double, "underfrequency_limit[Hz]", PADDR(underfrequency_limit), PT_DESCRIPTION, "Maximum positive input limit to PID controller is f0 - underfreuqnecy_limit",
			PT_double, "overfrequency_limit[Hz]", PADDR(overfrequency_limit), PT_DESCRIPTION, "Maximum negative input limit to PID controller is f0 - overfrequency_limit",
			PT_double, "deadband[Hz]", PADDR(deadband), PT_DESCRIPTION, "Deadpand for PID controller input in Hz",
			//PID controller
			PT_double, "PIDw_0[MW/Hz]", PADDR(PIDw[0]), PT_ACCESS, PA_HIDDEN, PT_DESCRIPTION, "PID Weight for current frequency error in MW/Hz",
			PT_double, "PIDw_1[MW/Hz]", PADDR(PIDw[1]), PT_ACCESS, PA_HIDDEN, PT_DESCRIPTION, "PID Weight for previous timestep frequency error in MW/Hz",
			PT_double, "PIDw_2[MW/Hz]", PADDR(PIDw[2]), PT_ACCESS, PA_HIDDEN, PT_DESCRIPTION, "PID Weight for t-2 frequency error in MW/Hz",
			PT_double, "kpPID[MW/Hz]", PADDR(kpPID), PT_DESCRIPTION, "PID proportional gain in MW/Hz",
			PT_double, "kiPID[MW/Hz/s]", PADDR(kiPID), PT_DESCRIPTION, "PID integral gain in MW/Hz/s",
			PT_double, "kdPID[MW/Hz*s]", PADDR(kdPID), PT_DESCRIPTION, "PID derivative gain in MW/Hz*s",
			
			PT_double, "Ts[s]", PADDR(Ts), PT_DESCRIPTION, "Secondary controller sampling period in sec.",
			
			PT_double, "alpha_tol", PADDR(alpha_tol), "Tolerance for participation values not summing to exactly one.",

			PT_bool, "sampleflag", PADDR(sampleflag), PT_ACCESS, PA_HIDDEN, PT_DESCRIPTION, "Booling flag whether output should be published or not by secondary controller",
			PT_double, "sample_time", PADDR(sample_time), PT_ACCESS, PA_HIDDEN, PT_DESCRIPTION, "Next update time for the secondary controller",

			//States
			PT_double, "deltaf(t)[Hz]", PADDR(curr_state.deltaf[0]), PT_ACCESS, PA_HIDDEN, PT_DESCRIPTION, "Frequency error in current iteration [Hz]",
			PT_double, "deltaf(t-1)[Hz]", PADDR(curr_state.deltaf[1]), PT_ACCESS, PA_HIDDEN, PT_DESCRIPTION, "Frequency error in previous timestep [Hz]",
			PT_double, "dxi[MW]", PADDR(curr_state.dxi), PT_ACCESS, PT_HIDDEN, PT_DESCRIPTION, "Change in PID integrator output",
			PT_double, "xi[MW]", PADDR(curr_state.xi), PT_ACCESS, PT_HIDDEN, PT_DESCRIPTION, "PID integrator output",
			PT_double, "PIDout[MW]", PADDR(curr_state.PIDout), PT_ACCESS, PT_HIDDEN, PT_DESCRIPTION, "PID output",
			NULL) < 1)
				GL_THROW("unable to publish properties in %s", __FILE__);

		defaults = this;

		memset(this, 0, sizeof(sec_control));

		//**************** Function publish for deltamode - postupdate may not be needed ****************//
		if (gl_publish_function(oclass, "preupdate_gen_object", (FUNCTIONADDR)preupdate_sec_control) == NULL)
			GL_THROW("Unable to publish sec_control deltamode function");
		if (gl_publish_function(oclass, "interupdate_gen_object", (FUNCTIONADDR)interupdate_sec_control) == NULL)
			GL_THROW("Unable to publish sec_control deltamode function");
		if (gl_publish_function(oclass, "postupdate_gen_object", (FUNCTIONADDR)postupdate_sec_control) == NULL)
			GL_THROW("Unable to publish sec_control deltamode function");
	}
}

//Isa function for identification
int sec_control::isa(char *classname)
{
	return strcmp(classname,"sec_control")==0;
}

/* Object creation is called once for each object that is created by the core */
int sec_control::create(void)
{
	////////////////////////////////////////////////////////
	// DELTA MODE
	////////////////////////////////////////////////////////
	deltamode_inclusive = false; //By default, don't be included in deltamode simulations
	first_sync_delta_enabled = false;

	//**************** created/default values go here ****************//
	test_published_property = complex(-1.0,-1.0);	//Flag value for example

	return 1; /* return 1 on success, 0 on failure */
}

/* Object initialization is called once after all object have been created */
int sec_control::init(OBJECT *parent)
{
	OBJECT *obj = OBJECTHDR(this);

	//Deferred initialization code
	if (parent != NULL)
	{
		if ((parent->flags & OF_INIT) != OF_INIT)
		{
			char objname[256];
			gl_verbose("sec_control::init(): deferring initialization on %s", gl_name(parent, objname, 255));
			return 2; // defer
		}
	}

	//Set the deltamode flag, if desired
	if ((obj->flags & OF_DELTAMODE) == OF_DELTAMODE)
	{
		deltamode_inclusive = true; //Set the flag and off we go
	}


	///////////////////////////////////////////////////////////////////////////
	// DELTA MODE
	///////////////////////////////////////////////////////////////////////////
	//See if we desire a deltamode update (module-level)
	if (deltamode_inclusive)
	{
		//Check global, for giggles
		if (enable_subsecond_models != true)
		{
			gl_warning("sec_control:%s indicates it wants to run deltamode, but the module-level flag is not set!", obj->name ? obj->name : "unnamed");
			/*  TROUBLESHOOT
			The sec_control object has the deltamode_inclusive flag set, but not the module-level enable_subsecond_models flag.  The generator
			will not simulate any dynamics this way.
			*/
		}
		else
		{
			gen_object_count++; //Increment the counter
			first_sync_delta_enabled = true;
		}
		//Default else - don't do anything
	}	 //End deltamode inclusive
	else //This particular model isn't enabled
	{
		if (enable_subsecond_models == true)
		{
			gl_warning("sec_control:%d %s - Deltamode is enabled for the module, but not this controller!", obj->id, (obj->name ? obj->name : "Unnamed"));
			/*  TROUBLESHOOT
			The sec_control is not flagged for deltamode operations, yet deltamode simulations are enabled for the overall system.  When deltamode
			triggers, this sec_control may no longer contribute to the system, until event-driven mode resumes.  This could cause issues with the simulation.
			It is recommended all objects that support deltamode enable it.
			*/
		}
		
	}

	//**************** Other initialization or "input check" items ****************//
	//Example check
	if (test_published_property.Re() == -1.0)	//Not set in GLM, so apply a default
	{
		//Arbitrary
		test_published_property = complex(1000.0,1000.0);
	}

	return 1;
}

TIMESTAMP sec_control::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	OBJECT *obj = OBJECTHDR(this);

	//**************** Initial top-down pass for QSTS - typically empty accumulators and re-init working variable for current pass ****************//

	return t2;
}

TIMESTAMP sec_control::sync(TIMESTAMP t0, TIMESTAMP t1)
{
	OBJECT *obj = OBJECTHDR(this);
	TIMESTAMP tret_value;

	//Assume always want TS_NEVER
	tret_value = TS_NEVER;

	//Deltamode check/init items
	if (first_sync_delta_enabled == true) //Deltamode first pass
	{
		//TODO: LOCKING!
		if ((deltamode_inclusive == true) && (enable_subsecond_models == true)) //We want deltamode - see if it's populated yet
		{
			if (((gen_object_current == -1) || (delta_objects == NULL)) && (enable_subsecond_models == true))
			{
				//Call the allocation routine
				allocate_deltamode_arrays();
			}

			//Check limits of the array
			if (gen_object_current >= gen_object_count)
			{
				GL_THROW("Too many objects tried to populate deltamode objects array in the generators module!");
				/*  TROUBLESHOOT
				While attempting to populate a reference array of deltamode-enabled objects for the generator
				module, an attempt was made to write beyond the allocated array space.  Please try again.  If the
				error persists, please submit a bug report and your code via the trac website.
				*/
			}

			//Add us into the list
			delta_objects[gen_object_current] = obj;

			//Map up the function for interupdate
			delta_functions[gen_object_current] = (FUNCTIONADDR)(gl_get_function(obj, "interupdate_gen_object"));

			//Make sure it worked
			if (delta_functions[gen_object_current] == NULL)
			{
				GL_THROW("Failure to map deltamode function for device:%s", obj->name);
				/*  TROUBLESHOOT
				Attempts to map up the interupdate function of a specific device failed.  Please try again and ensure
				the object supports deltamode.  If the error persists, please submit your code and a bug report via the
				trac website.
				*/
			}

			/* post_delta_functions are often removed, since they don't typically do anything - empty it out/delete it if this is the case! */
			//Map up the function for postupdate
			post_delta_functions[gen_object_current] = (FUNCTIONADDR)(gl_get_function(obj, "postupdate_gen_object"));

			//Make sure it worked
			if (post_delta_functions[gen_object_current] == NULL)
			{
				GL_THROW("Failure to map post-deltamode function for device:%s", obj->name);
				/*  TROUBLESHOOT
				Attempts to map up the postupdate function of a specific device failed.  Please try again and ensure
				the object supports deltamode.  If the error persists, please submit your code and a bug report via the
				trac website.
				*/
			}

			//Map up the function for postupdate
			delta_preupdate_functions[gen_object_current] = (FUNCTIONADDR)(gl_get_function(obj, "preupdate_gen_object"));

			//Make sure it worked
			if (delta_preupdate_functions[gen_object_current] == NULL)
			{
				GL_THROW("Failure to map pre-deltamode function for device:%s", obj->name);
				/*  TROUBLESHOOT
				Attempts to map up the preupdate function of a specific device failed.  Please try again and ensure
				the object supports deltamode.  If the error persists, please submit your code and a bug report via the
				trac website.
				*/
			}

			//Update pointer
			gen_object_current++;

			//Flag it
			first_sync_delta_enabled = false;

		}	 //End deltamode specials - first pass
		else //Somehow, we got here and deltamode isn't properly enabled...odd, just deflag us
		{
			first_sync_delta_enabled = false;
		}
	} //End first delta timestep
	//default else - either not deltamode, or not the first timestep

	//**************** Bottom-up execution events for QSTS - usually the actual "logic" before a powerflow call ****************//

	//Return
	if (tret_value != TS_NEVER)
	{
		return -tret_value;
	}
	else
	{
		return TS_NEVER;
	}
}

/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP sec_control::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	OBJECT *obj = OBJECTHDR(this);
	TIMESTAMP t2 = TS_NEVER; //By default, we're done forever!

	//**************** second top-down call - items that need to be reconciled after powerflow has executed ***************//

	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF DELTA MODE
//////////////////////////////////////////////////////////////////////////
//Preupdate
STATUS sec_control::pre_deltaupdate(TIMESTAMP t0, unsigned int64 delta_time)
{
	//**************** Deltamode start initialization function, if any ****************//
	sample_time = (double)t0 + Ts; //
	sampleflag = false;

	for (auto & obj : part_obj){
		//zero derivative terms in initialization
		for (i=0, i<2, i++){
			obj.dP[i] = 0;
			obj.dPP[i] = 0;
		}
	}
	//Just return a pass - not sure how we'd fail
	return SUCCESS;
}

//Module-level call
SIMULATIONMODE sec_control::inter_deltaupdate(unsigned int64 delta_time, unsigned long dt, unsigned int iteration_count_val)
{
	double deltat, deltath;
	OBJECT *obj = OBJECTHDR(this);

	SIMULATIONMODE simmode_return_value = SM_EVENT;

	//Get timestep value - used for predictor/corrector calculations
	deltat = (double)dt / (double)DT_SECOND;
	deltath = deltat / 2.0;

	//Update time tracking variables
	curr_timestamp_dbl = gl_globaldeltaclock;
	
	//**************** Primary deltamode executor - dynamics usually go/get called here ****************//

	/* Publish values only on first itration.
	Since the secondary controller is placed *before* all other participating objects
	the effect of this, is that for all participating objects it is as if the change was
	published at the end of the last iteration of the last timestep.
	*/
	if (iteration_count_val == 0) // Predictor pass
	{
		//cycle frequency error state values
		next_state.deltaf[1] = curr_state.deltaf[0];
		memcpy(&curr_state, &next_state, sizeof(SEC_CNTRL_STATE);
		
		//Determine wheter to pass set point changes on this timestep or not
		if (curr_timestamp_dbl >= sample_time)
		{
			sampleflag = true; 	//sample output on this timestep
			sample_time += Ts;  // set next timestep to sample
		}
		else
		{
			sampleflag = false;
		}

		// Get Frequency Deviation (sets curr_state.deltaf[0])
		get_deltaf(f_meas); //@Frank: how do I get f_meas?

		//PID Controller
		curr_state.dxi = kiPID*curr_state.deltaf[0];
		next_state.xi = curr_state.xi + deltat*curr_state.dxi //integrator output
		next_state.PIDout = next_state.xi + (kpPID + kdPID/deltat)*curr_state.deltaf[0] - kdPID/deltat*curr_state.deltaf[1];
		// End PID controller

		
		//iterate over participating objects
		for (auto & obj : part_obj){
			if (obj.Tlp < deltat)
			{
				gl_warning("sec_control: lowpass filter time constant for %s is less than deltat. Using deltat instead.", obj.name : "unnamed object");
				obj.Tlp = deltat;
			}
			// update change for ieach participant based on participation factor alpha and low pass filter.
			obj.ddP[0] = 1/obj.Tlp*(obj.alpha*next_state.PIDout - obj.dP[0]);
			obj.dP[1] = obj.dP[0] + deltat*obj.ddP[0];
			if (sampleflag)
			{
				//***************code to update object setpoints********************
			}
		}
		simmode_return_value = SM_DELTA_ITER;
	}
	else if (iteration_count_val == 1) // corrector pass
	{
		// Get Frequency Deviation (sets curr_state.deltaf[0])
		get_deltaf(f_meas); //@Frank: how do I get f_meas?
		
		//PID Controller
		next_state.dxi = kiPID*curr_state.deltaf[0];
		next_state.xi = curr_state.xi + deltath*(curr_state.dxi + next_state.dxi); //integrator output
		next_state.PIDout = next_state.xi + (kpPID + kdPID/deltat)*curr_state.deltaf[0] - kdPID/deltat*curr_state.deltaf[1];
		// End PID controller

		//iterate over participating objects
		for (auto & obj : part_obj){
			// update change for ieach participant based on participation factor alpha and low pass filter.
			obj.ddP[1] = 1/obj.Tlp*(obj.alpha*next_state.PIDout - obj.dP[1]);
			obj.dP[0] += deltath*(obj.ddP[0] + obj.ddP[1]);
			if (sampleflag)
			{
				//***************code to update object setpoints********************
				// The actually delta we want to pass is obj.dP[0] - obj.dP[1]
				// since obj.dP[1] was already passed in the predictor iteration
			}
		}

		simmode_return_value =  SM_DELTA;
	}
	else
	{
		// Do nothing
		simmode_return_value = SM_DELTA;
	}
	
	return simmode_return_value;
}

// Get Frequency Deviation including limiting and deadband handling.
// Functinalized since identical for any pass.
void sec_control::get_deltaf(double f_meas)
{
	curr_state.deltaf[0] = f0 - f_meas; // frequency error [Hz] 
	if (curr_state.deltaf[0] > (f0 - underfrequency_limit)) // limit maximum positive deviation (underfrequency)
	{
		curr_state.deltaf[0] = f0 - underfrequency_limit;
	}
	else if (curr_state.deltaf[0] < (f0 - overfrequency_limit)) //limit maximum negative deviation (overfrequency)
	{
		curr_state.deltaf[0] = f0 - overfrequency_limit;
	}
	else if (abs(curr_state.deltaf[0]) < deadband) //frequency is within deadband
	{
		curr_state.deltaf[0] = 0;
	}

	return SUCCESS; //Always succeeds.
}

STATUS sec_control::check_alpha(void)
{	
	double asum = 0; //alpha accumulator
	for (auto & obj : part_obj){
		asum += obj.alpha;
	}
	if (abs(asum - 1) <= alpha_tol)
	{
		return SUCCESS; // alphas sum to 1 (within tolerance)
	}
	else
	{
		return FAILED;
	}
} 


void sec_control::parse_objs(char* input)
{
	/***** Parse string to create an return an array ******/
	// Format should be:
	// ADD name1, alpha1, dp_min1, dp_max1,Tlp1; name2, alpha2, ...
	// REMOVE name1; name2; name3 ...
	// MODIFY name1, alpha1, dp_min1, dp_max1, Tlp1; name2, alpha2, ....
	// for the ADD key word we need all parameters passed.
	// for the MODIFY key word, it would be nice if we could pass only some
	// for example: MODIFY name1, alpha1, , , ; name2, , dp_min1, , ; ...

	// TODO: @Frank, i'd appreciate some help here :)

	// for the following it is assumed that the parsing returns
	// a vector of vectors something like 
	// [[name, alpha, dp_min, dp_max, Tlp], [...]] 
	for (auto elem : parsed_elems){
		SEC_CNTRL_PARTICIPANT tmpobj;
		for (int i=0, i<5, i++){
			if (i == 0){
				tmpobj.name = elem[i];
				// Does it mae more sense to put these in a different call in init?
				tmpobj.ptr = get_gld_obj(tmpobj.name); //somehow get the object pointer
				tmpobj.pset= get_pset(tmpobj.ptr); // pset should be defined based on object type/class
			}
			else if (i == 1){
				tmpobj.alpha = elem[i]
			}
			else if (i == 2){
				tmpobj.dp_min = elem[i]
			}
			else if (i == 3){
				tmpobj.dp_max = elem[i]
			}
			else if (i == 4){
				tmpobj.Tlp = elem[i]
			}
		}
	}
}



//Module-level post update call
/* This is just put here as an example - not sure what it would do */
STATUS sec_control::post_deltaupdate(complex *useful_value, unsigned int mode_pass)
{
	//**************** Post-deltamode transition back to QSTS items ****************//

	return SUCCESS; //Always succeeds right now
}

//Map Complex value
gld_property *sec_control::map_complex_value(OBJECT *obj, char *name)
{
	gld_property *pQuantity;
	OBJECT *objhdr = OBJECTHDR(this);

	//Map to the property of interest
	pQuantity = new gld_property(obj, name);

	//Make sure it worked
	if ((pQuantity->is_valid() != true) || (pQuantity->is_complex() != true))
	{
		GL_THROW("sec_control:%d %s - Unable to map property %s from object:%d %s", objhdr->id, (objhdr->name ? objhdr->name : "Unnamed"), name, obj->id, (obj->name ? obj->name : "Unnamed"));
		/*  TROUBLESHOOT
		While attempting to map a quantity from another object, an error occurred in the secondary controller.  Please try again.
		If the error persists, please submit your system and a bug report via the ticketing system.
		*/
	}

	//return the pointer
	return pQuantity;
}

//Map double value
gld_property *sec_control::map_double_value(OBJECT *obj, char *name)
{
	gld_property *pQuantity;
	OBJECT *objhdr = OBJECTHDR(this);

	//Map to the property of interest
	pQuantity = new gld_property(obj, name);

	//Make sure it worked
	if ((pQuantity->is_valid() != true) || (pQuantity->is_double() != true))
	{
		GL_THROW("sec_control:%d %s - Unable to map property %s from object:%d %s", objhdr->id, (objhdr->name ? objhdr->name : "Unnamed"), name, obj->id, (obj->name ? obj->name : "Unnamed"));
		/*  TROUBLESHOOT
		While attempting to map a quantity from another object, an error occurred in the secondary controller.  Please try again.
		If the error persists, please submit your system and a bug report via the ticketing system.
		*/
	}

	//return the pointer
	return pQuantity;
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////

EXPORT int create_sec_control(OBJECT **obj, OBJECT *parent)
{
	try
	{
		*obj = gl_create_object(sec_control::oclass);
		if (*obj != NULL)
		{
			sec_control *my = OBJECTDATA(*obj, sec_control);
			gl_set_parent(*obj, parent);
			return my->create();
		}
		else
			return 0;
	}
	CREATE_CATCHALL(sec_control);
}

EXPORT int init_sec_control(OBJECT *obj, OBJECT *parent)
{
	try
	{
		if (obj != NULL)
			return OBJECTDATA(obj, sec_control)->init(parent);
		else
			return 0;
	}
	INIT_CATCHALL(sec_control);
}

EXPORT TIMESTAMP sync_sec_control(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	sec_control *my = OBJECTDATA(obj, sec_control);
	try
	{
		switch (pass)
		{
		case PC_PRETOPDOWN:
			t2 = my->presync(obj->clock, t1);
			break;
		case PC_BOTTOMUP:
			t2 = my->sync(obj->clock, t1);
			break;
		case PC_POSTTOPDOWN:
			t2 = my->postsync(obj->clock, t1);
			break;
		default:
			GL_THROW("invalid pass request (%d)", pass);
			break;
		}
		if (pass == clockpass)
			obj->clock = t1;
	}
	SYNC_CATCHALL(sec_control);
	return t2;
}

EXPORT int isa_sec_control(OBJECT *obj, char *classname)
{
	return OBJECTDATA(obj,sec_control)->isa(classname);
}

EXPORT STATUS preupdate_sec_control(OBJECT *obj, TIMESTAMP t0, unsigned int64 delta_time)
{
	sec_control *my = OBJECTDATA(obj, sec_control);
	STATUS status_output = FAILED;

	try
	{
		status_output = my->pre_deltaupdate(t0, delta_time);
		return status_output;
	}
	catch (char *msg)
	{
		gl_error("preupdate_sec_control(obj=%d;%s): %s", obj->id, (obj->name ? obj->name : "unnamed"), msg);
		return status_output;
	}
}

EXPORT SIMULATIONMODE interupdate_sec_control(OBJECT *obj, unsigned int64 delta_time, unsigned long dt, unsigned int iteration_count_val)
{
	sec_control *my = OBJECTDATA(obj, sec_control);
	SIMULATIONMODE status = SM_ERROR;
	try
	{
		status = my->inter_deltaupdate(delta_time, dt, iteration_count_val);
		return status;
	}
	catch (char *msg)
	{
		gl_error("interupdate_sec_control(obj=%d;%s): %s", obj->id, obj->name ? obj->name : "unnamed", msg);
		return status;
	}
}

EXPORT STATUS postupdate_sec_control(OBJECT *obj, complex *useful_value, unsigned int mode_pass)
{
	sec_control *my = OBJECTDATA(obj, sec_control);
	STATUS status = FAILED;
	try
	{
		status = my->post_deltaupdate(useful_value, mode_pass);
		return status;
	}
	catch (char *msg)
	{
		gl_error("postupdate_sec_control(obj=%d;%s): %s", obj->id, obj->name ? obj->name : "unnamed", msg);
		return status;
	}
}
