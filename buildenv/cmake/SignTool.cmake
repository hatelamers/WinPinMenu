find_program(SIGNTOOL_COMMAND signtool)
if(SIGNTOOL_COMMAND)
    if(SIGNER_ISSUER)
        set(SIGNTOOL_OBJECT_ARG "/i" "${SIGNER_ISSUER}")
    elseif(SIGNER_CERT)
        set(SIGNTOOL_OBJECT_ARG "/f" "${SIGNER_CERT}")
    endif()
    if(SIGNER_PASS)
        set(SIGNTOOL_PASS_ARG "/p" "${SIGNER_PASS}")
    endif()
    if(SIGNTOOL_OBJECT_ARG)
        message(STATUS "Applying ${SIGNTOOL_OBJECT_ARG} to ${TARGET}")
        execute_process(
            COMMAND "${SIGNTOOL_COMMAND}"
                sign
                /tr http://timestamp.digicert.com
                /td SHA256
                /fd SHA256
                /v
                ${SIGNTOOL_OBJECT_ARG}
                ${SIGNTOOL_PASS_ARG}
                "${TARGET}"
        )
    else()
        message(STATUS "Neither SIGNER_ISSUER nor SIGNER_CERT provided, skipping signing")
    endif()
else()
    message(WARNING "signtool not found, skipping signing")
endif()