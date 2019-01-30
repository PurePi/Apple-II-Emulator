        ; interrupt vector table

        ORG $FFFA

        DFW $0200       ; NMI
        DFW $F800       ; RESET
        DFW $0200       ; IRQ
