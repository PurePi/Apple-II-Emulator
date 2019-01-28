        ORG $FA00

ACC     EQU $00
XREG    EQU $01
YREG    EQU $02
STATUS  EQU $03
SPTR    EQU $04
COUTSWL EQU $05
COUTSWH EQU $06

RESET   LDA #>COUT1
        STA COUTSWL
        LDA #<COUT1
        STA COUTSWH
        LDA #$81
        LDY #$02
        JSR COUT
        HCF
COUT    JMP (COUTSWL)
COUT1   STA $0400,Y
        RTS

SAVE    STA ACC
        STX XREG
        STY YREG
        PHP
        PLA
        STA STATUS
        TSX
        STX SPTR
        CLD
        RTS
RESTORE LDA STATUS
        PHA
        LDA ACC
        LDX XREG
        LDY YREG
        PLP
        RTS
