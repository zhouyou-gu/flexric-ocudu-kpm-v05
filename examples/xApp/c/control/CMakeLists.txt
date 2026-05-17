if(XAPP_DB STREQUAL "MYSQL_XAPP")
	include_directories(${LIBMYSQLCLIENT_INCLUDE_DIRS})
endif()

# xapp_mac_ctrl
add_executable(xapp_mac_ctrl
								xapp_mac_ctrl.c
								${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
								)

target_link_libraries(xapp_mac_ctrl
											PUBLIC
											e42_xapp
											$<TARGET_OBJECTS:e2_time_obj>
											$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
											-pthread
											-lsctp
											-ldl
											)

# xapp_slice_moni_ctrl
add_executable(xapp_slice_moni_ctrl
		xapp_slice_moni_ctrl.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
)

target_link_libraries(xapp_slice_moni_ctrl
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
)

# xapp_oran_ctrl
add_executable(xapp_oran_ctrl
		xapp_oran_ctrl.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
		)

target_link_libraries(xapp_oran_ctrl
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)

# OCUDU benchmark one-shot E2SM-CCC PRB policy control tool.
add_executable(ocudu_ccc_prb_control
		ocudu_ccc_prb_control.c
		ocudu_prb_control_common.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
		)
set_target_properties(ocudu_ccc_prb_control PROPERTIES OUTPUT_NAME ocudu-ccc-prb-control)
target_link_libraries(ocudu_ccc_prb_control
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)

# OCUDU benchmark one-shot E2SM-RC DU slice PRB quota control tool.
enable_language(CXX)
set(OCUDU_ASN1_ROOT "$ENV{OCUDU_ASN1_ROOT}" CACHE PATH "Path to OCUDU source subset containing generated ASN.1 C++ support")
if(NOT OCUDU_ASN1_ROOT)
	set(OCUDU_ASN1_ROOT "/opt/ocudu-asn1")
endif()
if(NOT EXISTS "${OCUDU_ASN1_ROOT}/include/ocudu/asn1/e2sm/e2sm_rc_ies.h")
	message(FATAL_ERROR "OCUDU_ASN1_ROOT must point to OCUDU generated ASN.1 sources for the RC DU control tool")
endif()

add_executable(ocudu_rc_du_prb_control
			ocudu_rc_du_prb_control.cpp
			ocudu_prb_control_common.c
			${OCUDU_ASN1_ROOT}/lib/asn1/e2sm/e2sm_kpm_ies.cpp
			${OCUDU_ASN1_ROOT}/lib/asn1/e2sm/e2sm_rc_ies.cpp
			${OCUDU_ASN1_ROOT}/lib/asn1/e2sm/e2sm_common_ies.cpp
			${OCUDU_ASN1_ROOT}/lib/asn1/asn1_utils.cpp
			${OCUDU_ASN1_ROOT}/lib/support/byte_buffer.cpp
			${OCUDU_ASN1_ROOT}/external/fmt/src/format.cc
			${OCUDU_ASN1_ROOT}/lib/ocudulog/ocudulog.cpp
			${OCUDU_ASN1_ROOT}/lib/ocudulog/ocudulog_c.cpp
			${OCUDU_ASN1_ROOT}/lib/ocudulog/backend_worker.cpp
			${OCUDU_ASN1_ROOT}/lib/ocudulog/event_trace.cpp
			${OCUDU_ASN1_ROOT}/lib/ocudulog/formatters/json_formatter.cpp
			${OCUDU_ASN1_ROOT}/lib/ocudulog/formatters/text_formatter.cpp
			${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
			)
set_target_properties(ocudu_rc_du_prb_control PROPERTIES OUTPUT_NAME ocudu-rc-du-prb-control)
target_include_directories(ocudu_rc_du_prb_control
			PUBLIC
			${OCUDU_ASN1_ROOT}/include
			${OCUDU_ASN1_ROOT}/external
			${OCUDU_ASN1_ROOT}/external/fmt/include
			${OCUDU_ASN1_ROOT}/include/ocudu/ocudulog/bundled
			${OCUDU_ASN1_ROOT}/include/ocudu/ocudulog/formatters
			${OCUDU_ASN1_ROOT}/lib/ocudulog
			${OCUDU_ASN1_ROOT}/lib/ocudulog/formatters
			${OCUDU_ASN1_ROOT}/lib/ocudulog/sinks
			)
target_link_libraries(ocudu_rc_du_prb_control
			PUBLIC
			e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)
