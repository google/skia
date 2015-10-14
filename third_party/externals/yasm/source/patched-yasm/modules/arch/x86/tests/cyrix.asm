[bits 32]
        svdc    tword [2*eax+esi+12345678h],cs
        svdc    tword [2*eax+esi+12345678h],ds
        svdc    [2*eax+esi+12345678h],es

        rsdc    cs,tword [2*eax+esi+12345678h]
        rsdc    ds,tword [2*eax+esi+12345678h]
        rsdc    es,[2*eax+esi+12345678h]

        wrshr   eax
        wrshr   dword [4*edx+esi+12345678h]
        wrshr   [4*edx+esi+12345678h]

        rdshr   eax
        rdshr   dword [4*edx+esi+12345678h]
        rdshr   [4*edx+esi+12345678h]

