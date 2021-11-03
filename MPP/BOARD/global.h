#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#define _CONCAT2(a,b) a##b
#define CONCAT2(a,b) _CONCAT2(a,b)

//#define _CONCAT3(a,b,c) a##b##c
//#define CONCAT3(a,b,c) _CONCAT3(a,b,c)



//#define O3 __attribute__((optimize("O3")))
//#define INLINE __attribute__((always_inline)) inline

#define O3
#define INLINE


// Systemtick (jede Millisekunde)
extern unsigned long systickcounter;

// Flags, die Aufgaben innerhalb der Mainloop anstoﬂen (Entlastung der Interrupts)
extern unsigned int mainloopflags;


#endif
