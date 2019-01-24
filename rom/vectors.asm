        ; interrupt vector table

        ORG $FFFA

        DFW $0200       ; NMI
        DFW $FA00       ; RESET
        DFW $0200       ; IRQ