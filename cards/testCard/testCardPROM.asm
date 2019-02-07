        ; sample PROM to demonstrate functionality
        ; address-independent in this example, so no ORG needed

SOMWHER EQU $F8F7       ; some absolutely addresses subroutine somewhere else in memory, which ideally would do something useful
                        ; we know that a subroutine (just RTS right now) is at this address by generating and looking at examples_listing.txt

        JSR SOMWHER     ; JSR somewhere to push the current page onto the stack
        SEI             ; disable interrupts
        TSX             ; stack pointer points to page of return address that JSR just pushed, slot n is in the form of $Cn
        LDA $0100,X     ; load that page number into A
        STA $07F8       ; $7F8 is designated to keep page number of active card
        AND #$0F
        TAY             ; put the slot number as $0n int Y (will use this later)
        ASL A
        ASL A
        ASL A
        ASL A
        TAX             ; X now has slot number in the form of $n0
        LSR A
        LSR A
        LSR A
        LSR A           ; A now has the slot number as $0n again
        STA $C080,X     ; put it in byte 0 of your card's GPIO space to tell the card which slot it's in
        LDA $C084,X     ; read from byte 4 of your card's GPIO space (which the card will always put 42 into) (can go up to $C08F,X for byte 15)
        STA $0478,Y     ; put it into byte 0 of scratchpad
        STA $04F8,Y     ; also into byte 1 of scratchpad (the rest are $0578,Y, $05F8,Y, ... up to $07F8,Y)
        JSR $C800       ; can now safely access your card's expansion rom ($C800-$CFFF)
        RTS             ; PROM should always be JSR'ed to, so we RTS when it's done