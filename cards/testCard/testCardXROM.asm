        ; sample XROM

        ; XROM is absolutely addresses, so we do ORG
        ORG $C800

        LDA #$3F        ; just have whatever subroutines that don't fit into PROM here
        STA $C081,X
        LDX #$54
        RTS