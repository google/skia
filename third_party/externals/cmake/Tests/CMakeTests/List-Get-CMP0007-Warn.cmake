set(thelist "" NEW OLD)

list(GET thelist 1 thevalue)
if (NOT thevalue STREQUAL "OLD")
    message(SEND_ERROR "returned element '${thevalue}', but expected 'OLD'")
endif()
