.arm
.align 4

.global __svcGetCurrentProcessorNumber
.type __svcGetCurrentProcessorNumber, %function
__svcGetCurrentProcessorNumber:
	svc 0x11
	bx   lr
