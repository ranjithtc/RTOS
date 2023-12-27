;
 ;asm.s
 ;



   .def getPSP
   .def setPSP
   .def getMSP
   .def setASP
   .def setPrivilege
   .def setunPrivilege
   .def movPsp
   .def saveregs
   .def popregs
   .def xpsrtolr
   .def pushremainingone
   .def pushremainingtwo
   .def getsvc

.thumb

.text


getMSP:
      MRS R0,MSP
      BX LR



setASP:
; Read CONTROL register into R0
      MRS R0, CONTROL

; Set  ASP bit (bit 1) in R0( thehat will be the '10' in binary which is the second bit
      ORR R0, R0,#2

; updated value in control reg
      MSR CONTROL, R0
      ISB
      BX LR


setPSP:
      MSR PSP,R0
      BX LR




getPSP:
      MRS R0,PSP
      BX LR



movPsp:
         MRS R2, PSP

         SUB R2, #4; as we are going down the stack
         STR R0, [R2]
         MSR PSP, R2
         ISB
         BX LR



setunPrivilege:

               MRS R2, CONTROL

               ORR R2, R2,#1
                     ; TMPL set unprivileged in thread mode
               MSR CONTROL, R2

               ISB

               BX LR
setPrivilege:

    MRS R2, CONTROL      ; Read CONTROL register into R2

    AND R2, R2, #0 ; Clear bit 0 (TMPL) in R2

    MSR CONTROL, R2      ; Write R2 back to CONTROL register

    ISB                  ; Instruction Synchronization Barrier

    BX LR                ; Return from subroutine


popregs:

   MRS R0, PSP

   ;LDR LR, [R0]
   ;ADD R0, #4


    ;LDR R0, [R0] ; Pop the dummy value
    ;ADD R0, #4

       LDR R11,[R0]
       ADD R0,#4

       LDR R10,[R0]
       ADD R0,#4

       LDR R9,[R0]
       ADD R0,#4

       LDR R8,[R0]
       ADD R0,#4

       LDR R7,[R0]
       ADD R0,#4

       LDR R6,[R0]
       ADD R0,#4

       LDR R5,[R0]
       ADD R0,#4

       LDR R4,[R0]
       ADD R0,#4



       MSR PSP,R0

       BX LR



saveregs:
 MRS R1, PSP



       SUB R1,#4
       STR R4,[R1]

       SUB R1,#4
       STR R5,[R1]

       SUB R1,#4
       STR R6,[R1]

       SUB R1,#4
       STR R7,[R1]

       SUB R1,#4
       STR R8,[R1]

       SUB R1,#4
       STR R9,[R1]

       SUB R1,#4
       STR R10,[R1]

       SUB R1,#4
       STR R11,[R1]

   ; SUB R1, #4
   ; STR LR, [R1] ; Save LR on the stack

    ;SUB R1, #4
    ;STR R0, [R1] ; Push a dummy value to maintain 8-byte alignment

      MSR PSP,R1

      BX LR



xpsrtolr:
     MRS R3, PSP
     SUB R3,#4
     STR R0,[R3] ;xpsr

     SUB R3,#4
     STR R1,[R3]

     SUB R3,#4
     STR R2,[R3]


     MSR PSP,R3
     BX LR


pushremainingone:
     MRS R3, PSP
     SUB R3,#4
     STR R0,[R3] ;xpsr

     SUB R3,#4
     STR R1,[R3]

     SUB R3,#4
     STR R2,[R3]


     MSR PSP,R3
     BX LR



pushremainingtwo:
    MRS R3, PSP
    SUB R3,#4
    STR R0,[R3] ;xpsr

    SUB R3,#4
    STR R1,[R3]

    MSR PSP,R3

    BX LR


getsvc:
          MRS R0, PSP
; refered to https://www.iotality.com/armcm-svc/
          ADD R0,#24

          LDR R0,[R0]
          SUB R0,#2
          LDRB R0,[R0]; refered to https://www.iotality.com/armcm-svc/
          BX LR
