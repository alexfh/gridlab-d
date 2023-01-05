pkglib_LTLIBRARIES += powerflow/powerflow.la 

powerflow_powerflow_la_CPPFLAGS =
powerflow_powerflow_la_CPPFLAGS += -I$(top_srcdir)/third_party/superLU_MT
powerflow_powerflow_la_CPPFLAGS += -I$(top_srcdir)/third_party/jsonCpp
powerflow_powerflow_la_CPPFLAGS += $(AM_CPPFLAGS)

powerflow_powerflow_la_LDFLAGS =
powerflow_powerflow_la_LDFLAGS += $(AM_LDFLAGS)
powerflow_powerflow_la_LDFLAGS += -ldl

powerflow_powerflow_la_LIBADD =
powerflow_powerflow_la_LIBADD += third_party/jsonCpp/libjsoncpp.la
powerflow_powerflow_la_LIBADD += third_party/superLU_MT/libsuperlu.la
powerflow_powerflow_la_LIBADD += $(PTHREAD_CFLAGS)
powerflow_powerflow_la_LIBADD += $(PTHREAD_LIBS)

powerflow_powerflow_la_SOURCES =
powerflow_powerflow_la_SOURCES += powerflow/b2b_converter.cpp
powerflow_powerflow_la_SOURCES += powerflow/b2b_converter.h
powerflow_powerflow_la_SOURCES += powerflow/billdump.cpp
powerflow_powerflow_la_SOURCES += powerflow/billdump.h
powerflow_powerflow_la_SOURCES += powerflow/capacitor.cpp
powerflow_powerflow_la_SOURCES += powerflow/capacitor.h
powerflow_powerflow_la_SOURCES += powerflow/currdump.cpp
powerflow_powerflow_la_SOURCES += powerflow/currdump.h
powerflow_powerflow_la_SOURCES += powerflow/emissions.cpp
powerflow_powerflow_la_SOURCES += powerflow/emissions.h
powerflow_powerflow_la_SOURCES += powerflow/fault_check.cpp
powerflow_powerflow_la_SOURCES += powerflow/fault_check.h
powerflow_powerflow_la_SOURCES += powerflow/fuse.cpp
powerflow_powerflow_la_SOURCES += powerflow/fuse.h
powerflow_powerflow_la_SOURCES += powerflow/impedance_dump.cpp
powerflow_powerflow_la_SOURCES += powerflow/impedance_dump.h
powerflow_powerflow_la_SOURCES += powerflow/jsondump.cpp
powerflow_powerflow_la_SOURCES += powerflow/jsondump.h
powerflow_powerflow_la_SOURCES += powerflow/init.cpp
powerflow_powerflow_la_SOURCES += powerflow/line_configuration.cpp
powerflow_powerflow_la_SOURCES += powerflow/line_configuration.h
powerflow_powerflow_la_SOURCES += powerflow/line.cpp
powerflow_powerflow_la_SOURCES += powerflow/line.h
powerflow_powerflow_la_SOURCES += powerflow/line_spacing.cpp
powerflow_powerflow_la_SOURCES += powerflow/line_spacing.h
powerflow_powerflow_la_SOURCES += powerflow/link.cpp
powerflow_powerflow_la_SOURCES += powerflow/link.h
powerflow_powerflow_la_SOURCES += powerflow/load.cpp
powerflow_powerflow_la_SOURCES += powerflow/load.h
powerflow_powerflow_la_SOURCES += powerflow/load_tracker.cpp
powerflow_powerflow_la_SOURCES += powerflow/load_tracker.h
powerflow_powerflow_la_SOURCES += powerflow/main.cpp
powerflow_powerflow_la_SOURCES += powerflow/meter.cpp
powerflow_powerflow_la_SOURCES += powerflow/meter.h
powerflow_powerflow_la_SOURCES += powerflow/meter_test.h
powerflow_powerflow_la_SOURCES += powerflow/motor.cpp
powerflow_powerflow_la_SOURCES += powerflow/motor.h
powerflow_powerflow_la_SOURCES += powerflow/performance_motor.cpp
powerflow_powerflow_la_SOURCES += powerflow/performance_motor.h
powerflow_powerflow_la_SOURCES += powerflow/node.cpp
powerflow_powerflow_la_SOURCES += powerflow/node.h
powerflow_powerflow_la_SOURCES += powerflow/overhead_line_conductor.cpp
powerflow_powerflow_la_SOURCES += powerflow/overhead_line_conductor.h
powerflow_powerflow_la_SOURCES += powerflow/overhead_line.cpp
powerflow_powerflow_la_SOURCES += powerflow/overhead_line.h
powerflow_powerflow_la_SOURCES += powerflow/overheadline_test.h
powerflow_powerflow_la_SOURCES += powerflow/powerflow.cpp
powerflow_powerflow_la_SOURCES += powerflow/powerflow.h
powerflow_powerflow_la_SOURCES += powerflow/powerflow_library.cpp
powerflow_powerflow_la_SOURCES += powerflow/powerflow_library.h
powerflow_powerflow_la_SOURCES += powerflow/powerflow_object.cpp
powerflow_powerflow_la_SOURCES += powerflow/powerflow_object.h
powerflow_powerflow_la_SOURCES += powerflow/power_metrics.cpp
powerflow_powerflow_la_SOURCES += powerflow/power_metrics.h
powerflow_powerflow_la_SOURCES += powerflow/pqload.cpp
powerflow_powerflow_la_SOURCES += powerflow/pqload.h
powerflow_powerflow_la_SOURCES += powerflow/recloser.cpp		
powerflow_powerflow_la_SOURCES += powerflow/recloser.h
powerflow_powerflow_la_SOURCES += powerflow/regulator_configuration.cpp
powerflow_powerflow_la_SOURCES += powerflow/regulator_configuration.h
powerflow_powerflow_la_SOURCES += powerflow/regulator.cpp
powerflow_powerflow_la_SOURCES += powerflow/regulator.h
powerflow_powerflow_la_SOURCES += powerflow/regulator_test.h
powerflow_powerflow_la_SOURCES += powerflow/restoration.cpp
powerflow_powerflow_la_SOURCES += powerflow/restoration.h
powerflow_powerflow_la_SOURCES += powerflow/sectionalizer.cpp	
powerflow_powerflow_la_SOURCES += powerflow/sectionalizer.h
powerflow_powerflow_la_SOURCES += powerflow/series_compensator.cpp	
powerflow_powerflow_la_SOURCES += powerflow/series_compensator.h
powerflow_powerflow_la_SOURCES += powerflow/series_reactor.cpp
powerflow_powerflow_la_SOURCES += powerflow/series_reactor.h
powerflow_powerflow_la_SOURCES += powerflow/solver_nr.cpp
powerflow_powerflow_la_SOURCES += powerflow/solver_nr.h
powerflow_powerflow_la_SOURCES += powerflow/sync_check.cpp
powerflow_powerflow_la_SOURCES += powerflow/sync_check.h
powerflow_powerflow_la_SOURCES += powerflow/substation.cpp
powerflow_powerflow_la_SOURCES += powerflow/substation.h
powerflow_powerflow_la_SOURCES += powerflow/switch_object.cpp
powerflow_powerflow_la_SOURCES += powerflow/switch_object.h
powerflow_powerflow_la_SOURCES += powerflow/test.cpp
powerflow_powerflow_la_SOURCES += powerflow/test.h
powerflow_powerflow_la_SOURCES += powerflow/transformer_configuration.cpp
powerflow_powerflow_la_SOURCES += powerflow/transformer_configuration.h
powerflow_powerflow_la_SOURCES += powerflow/transformer.cpp
powerflow_powerflow_la_SOURCES += powerflow/transformer.h
powerflow_powerflow_la_SOURCES += powerflow/transformer_test.h
powerflow_powerflow_la_SOURCES += powerflow/triplex_line_conductor.cpp
powerflow_powerflow_la_SOURCES += powerflow/triplex_line_conductor.h
powerflow_powerflow_la_SOURCES += powerflow/triplex_line_configuration.cpp
powerflow_powerflow_la_SOURCES += powerflow/triplex_line_configuration.h
powerflow_powerflow_la_SOURCES += powerflow/triplex_line.cpp
powerflow_powerflow_la_SOURCES += powerflow/triplex_line.h
powerflow_powerflow_la_SOURCES += powerflow/triplexline_test.h
powerflow_powerflow_la_SOURCES += powerflow/triplex_load.h
powerflow_powerflow_la_SOURCES += powerflow/triplex_load.cpp
powerflow_powerflow_la_SOURCES += powerflow/triplex_meter.cpp
powerflow_powerflow_la_SOURCES += powerflow/triplex_meter.h
powerflow_powerflow_la_SOURCES += powerflow/triplex_node.cpp
powerflow_powerflow_la_SOURCES += powerflow/triplex_node.h
powerflow_powerflow_la_SOURCES += powerflow/underground_line_conductor.cpp
powerflow_powerflow_la_SOURCES += powerflow/underground_line_conductor.h
powerflow_powerflow_la_SOURCES += powerflow/underground_line.cpp
powerflow_powerflow_la_SOURCES += powerflow/underground_line.h
powerflow_powerflow_la_SOURCES += powerflow/undergroundline_test.h
powerflow_powerflow_la_SOURCES += powerflow/vfd.cpp
powerflow_powerflow_la_SOURCES += powerflow/vfd.h
powerflow_powerflow_la_SOURCES += powerflow/voltdump.cpp
powerflow_powerflow_la_SOURCES += powerflow/voltdump.h
powerflow_powerflow_la_SOURCES += powerflow/volt_var_control.cpp
powerflow_powerflow_la_SOURCES += powerflow/volt_var_control.h
