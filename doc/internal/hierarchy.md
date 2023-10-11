# Hierarchy

Up to [Documentation](README.md).


- __abc ::__
    - __i__
    - size.h
    - ascii.h
    - timestamp.h
    - util.h
    - buffer_streambuf.h
    - multifile_streambuf.h
    - stream.h
    - table_stream.h
    - __diag ::__
        - __i__
        - tag.h
        - log.h
        - exception.h
        - diag_ready.h
    - __test ::__
        - __i__
        - test.h
    - __net ::__
        - __i__
        - http.h
        - json.h __NEXT. TODO:__
        - socket.h
        - endpoint.h
        - socket_streambuf.h
        - __openssl ::__
            - __i__
            - socket.h
    - __vmem ::__
        - __i__
        - layout.h
        - pool.h
        - linked.h
        - container.h
        - iterator.h
        - list.h
        - map.h
        - string.h
        - util.h
        - string_streambuf.h
    - __gpio ::__
        - __i__
        - base.h
        - chip.h
        - line.h
        - ultrasonic.h
        - pwm_emulator.h
        - __smbus ::__
            - __i__
            - base.h
            - pwm.h
            - motor.h
            - servo.h
            - grayscale.h
            - motion.h
            - motion_tracker.h

## Dependency
A class may depend on other classes:
- from the same sub-namespace, or
- from sub-namespaces defined above its sub-namespace on the diagram