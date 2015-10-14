nop
end1:

line_out equ    end1+258        ;length = 1 + 263
real_end equ    end1+258+264

        cmp     bx,(real_end-line_out)/4
	cmp	bx,((end1+258+264)-(end1+258))/4
	cmp	bx,(end1+258+264-end1-258)/4
