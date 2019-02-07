        ORG $F800       we know this code will be loaded into address $F800, and the reset vector points here

ACC     EQU $00         zeropage addresses used for storing important data
XREG    EQU $01
YREG    EQU $02
STATUS  EQU #03
SPTR    EQU $04
COUTLO  EQU $05         for storing low/high byte of row address
COUTHI  EQU $06
PAGE1Y  EQU $07         keep track of Y position on first page
PAGE2Y  EQU $08         keep track of Y position on second page

        JMP print       jump to an example

*       writing to screen demo
*       The contents of memory from a cold start is all 0's,
*       which is displayed as the inverse '@', so that will be displayed until it's overwritten in memory

ODDROWL EQU $00         low byte of odd-numbered rows (first row is row 1 in this example)
EVNROWL EQU $80
ROW1H   EQU $04         high byte of rows 1 and 2
ROW3H   EQU $05         high byte of rows 3 and 4
PG2H    EQU $08         high byte of screen page 2 (first row)

print   LDA #$A0        start at first normal character
        LDY #$00        index in row to place char
        LDX #ROW1H      put address of row into COUTLO, COUTHI
        STX COUTHI
        LDX #ODDROWL
        STX COUTLO
        JSR outloop
        LDX #EVNROWL    adjust row base address, row 2 is even but page is the same so COUTHI can stay
        STX COUTLO
        JSR outloop
        LDX #ROW3H      adjust row base address, page also changes this time
        STX COUTHI
        LDX #ODDROWL
        STX COUTLO
        JSR outloop
done    HCF             halt and catch fire
outloop STA (COUTLO),Y  store character at row base address + Y
        CMP #$DF
        BEQ done        done when we reach last character
        ADC #$01
        INY
        CPY #$28
        BNE outloop
        LDY #$00
        CLC             CPY sets carry if Y >= data ($28 = $28 in this case)
        RTS

*       print keyboard input onto screen demo

kbd     LDY #$00        set up screen output the same way as above example
        LDX #ROW1H
        STX COUTHI
        LDX #ODDROWL
        STX COUTLO
undersc CLC
        LDA #$5F
        STA (COUTLO),Y
scan    BIT $C000       check if highest bit is on (input received flag)
        BMI input
        JMP scan
input   LDA $C000       get that input, keep highest bit to display normally
        STA $C010       reference $C010-$C01F to clear input flag on $C000-$C00F
        CMP #$8D        enter to switch pages
        BEQ swtchpg
        CLC
        STA (COUTLO),Y
        INY
        CPY #$28
        BNE undersc
        LDY #$00        just wrap around same line
        JMP undersc
swtchpg LDX COUTHI
        CPX #PG2H       if high byte of row is $08, we're currently on second page so we switch to the first
        BEQ firstpg     otherwise this branch doesn't happen and it continues to switch to second page
secndpg STA $C055       switch to second page
        STY PAGE1Y      save Y offset used in first page
        LDY PAGE2Y      recall the offset used in second page
        LDX #PG2H       switch base address of row to that of the second page
        STX COUTHI
        JMP undersc
firstpg STA $C054       switch to first page
        STY PAGE2Y
        LDY PAGE1Y
        LDX #ROW1H
        STX COUTHI
        JMP undersc

*       low resolution graphics demo

lores   LDY #$00
        LDX #ROW1H
        STX COUTHI
        LDX #ODDROWL
        STX COUTLO
        LDA #$00
        STA $C050       switch to graphics mode
        STA $C056       set lo-res
loloop  CLC
        STA (COUTLO),Y
        ADC #$01
        INY
        CMP #$28
        BNE loloop
        HCF

*       high resolution and mixed graphics demo

HIPG1L  EQU $00
HIPG1H  EQU $20

hires   LDY #$00
        LDX #HIPG1H
        STX COUTHI
        LDX #HIPG1L
        STX COUTLO
        STA $C050       switch to graphics mode (hi res already set from cold start)
        STA (COUTLO),Y  just placing a few random colors
        LDA #%00010010
        INY
        STA (COUTLO),Y
        LDA #%00000001  MSb is palette, then pairs from LSb determine color
        INY
        STA (COUTLO),Y
        LDA #%00100111  palette = 0, %00 = black, %01 = green, %10 = violet, %11 = white
        INY
        STA (COUTLO),Y
        LDA #%00001001  bit 6 is ignored
        INY
        STA (COUTLO),Y
        LDA #%00111111
        INY
        STA (COUTLO),Y
        LDA #%00111011  note that LSb is leftmost pixel
        INY
        STA (COUTLO),Y
        LDA #%10100101  palette = 1, %00 = black, %01 = orange, %10 = blue, %11 = white
        INY
        STA (COUTLO),Y
        LDA #%10110110
        INY
        STA (COUTLO),Y
        LDA #%10001100
        INY
        STA (COUTLO),Y
        LDA #%10100001
        INY
        STA (COUTLO),Y
        STA $C053       enable mixed mode
        LDA #$C1        place an 'A' at the first location for text captions
        STA $0650
        HCF

*       using peripheral cards in slot 1 and 4 example (both contain the same card)

card    JSR $C100       jump to slot 1's PROM
        JSR $C400       now slot 4
        JSR $C100       and slot 1 again
        HCF

        RTS             dummy subroutine, see testCardPROM.asm (this is where SOMWHER points to)
*                       if you change any code above this RTS, then its address will most likely move so
*                       then testCardPROM won't jump to it successfully because some other bytes will be at $F8F7
*                       I suggest writing new code after this line and changing line 13 to JMP down here if you
*                       choose to modify this file