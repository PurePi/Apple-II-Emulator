        ORG $FA00       ; we know this code will be loaded into address $FA00, and the reset vector points here

ACC     EQU $00         ; zeropage addresses used for storing important data
XREG    EQU $01
YREG    EQU $02
STATUS  EQU #03
SPTR    EQU $04
COUTLO  EQU $05         ; for storing low/high byte of row address
COUTHI  EQU $06
PAGE1Y  EQU $07         ; keep track of Y position on first page
PAGE2Y  EQU $08         ; keep track of Y position on second page

        JMP kbdex       ; jump to an example

*       writing to screen example

ODDROWL EQU $00         ; low byte of odd-numbered rows (first row is row 1 in this example)
EVNROWL EQU $80
ROW1H   EQU $04         ; high byte of rows 1 and 2
ROW3H   EQU $05         ; high byte of rows 3 and 4
PG2H    EQU $08         ; high byte of screen page 2 (first row)

printex LDA #$21        ; starting ascii code
        LDY #$00        ; index in row to place char

        LDX #ROW1H      ; put address of row into COUTLO, COUTHI
        STX COUTHI
        LDX #ODDROWL
        STX COUTLO
        JSR outloop

        LDX #EVNROWL    ; adjust row base address, row 2 is even but page is the same so COUTHI can stay
        STX COUTLO
        JSR outloop

        LDX #ROW3H      ; adjust row base address, page also changes this time
        STX COUTHI
        LDX #ODDROWL
        STX COUTLO
        JSR outloop
done    HCF             ; halt and catch fire

outloop STA (COUTLO),Y  ; store character at row base address + Y
        CMP #$7F
        BEQ done        ; done when we reach last character
        ADC #$01
        INY
        CPY #$28
        BNE outloop
        LDY #$00
        CLC             ; CPY sets carry if Y >= data ($28 = $28 in this case)
        RTS

*       print keyboard input onto screen example

kbdex   LDY #$00        ; set up screen output the same way as above example

        LDX #ROW1H
        STX COUTHI
        LDX #ODDROWL
        STX COUTLO

scanin  BIT $C000       ; check if highest bit is on (input received flag)
        BMI input
        JMP scanin

input   LDA $C000       ; get that input, remove highest bit to get ascii code
        AND #$7F
        STA $C010       ; reference $C010-$C01F to clear input flag on $C000-$C00F
        CMP #$09        ; tab to switch pages
        BEQ swtchpg
        CLC
        STA (COUTLO),Y
        INY
        CPY #$28
        BNE scanin
        LDY #$00        ; just wrap around same line
        CLC             ; CPY sets carry if Y >= data ($28 = $28 in this case)
        JMP scanin

swtchpg LDX COUTHI
        CPX #PG2H       ; if high byte of row is $08, we're currently on second page so we switch to the first
        BEQ firstpg
secndpg STA $C055       ; switch to second page
        STY PAGE1Y      ; save Y offset used in first page
        LDY PAGE2Y      ; recall the offset used in second page
        LDX #PG2H       ; switch base address of row to that of the second page
        STX COUTHI
        JMP scanin
firstpg STA $C054       ; switch to first page
        STY PAGE2Y
        LDY PAGE1Y
        LDX #ROW1H
        STX COUTHI
        JMP scanin

*       peripheral card PROM example
***     NOT YET IMPLEMENTED

        JSR SAVE        ; save all registers
        SEI             ; disable interrupts
        TSX             ; stack pointer points to page of return address that JSR just pushed, slot n is in the form of $Cn
        LDA $0100,X     ; load that page number into A
        STA $07F8       ; $7F8 is designated to keep page number of active card
        AND #$0F
        ASL A
        ASL A
        ASL A
        ASL A
        TAX             ; X now has slot number in the form of $n0
        LDA $C080,X     ; access byte 0 of your card's scratchpad
        LDA $C081,X     ; access byte 1 of your card's scratchpad (up to $C08F,X for byte 15)
        LDA $CFFF       ; request access to expansion rom by referencing $CFFF
        JSR $C800       ; can now access your card's expansion rom ($C800-$CFFF)

*       ROM subroutines

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