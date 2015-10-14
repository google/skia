#!/usr/bin/env python

def emit(opcode,suffix,width,order,optype):
    d = {}
    d['opcode']=opcode
    d['suffix']=suffix
    d['order']=order
    if width == 128:
        d['op1']= 'xmm1'
        d['op2']= 'xmm2'
        d['op3']= 'xmm3'
        if optype == 'rrr':
            d['op3']= 'xmm3'
        elif suffix == 'pd':
            d['op3']= 'dqword [rax]'
        elif suffix == 'sd':
            d['op3']= 'qword [rax]'
        elif suffix == 'ss':
            d['op3']= 'dword [rax]'
    else:
        d['op1']= 'ymm1'
        d['op2']= 'ymm2'
        if optype == 'rrr':
            d['op3']= 'ymm3'
        else:
            d['op3']= 'yword [rax]'


    print "v%(opcode)s%(order)s%(suffix)s %(op1)s, %(op2)s, %(op3)s" % (d)
    if optype == 'rrm':
        d['op3']= '[rax]'
        print "v%(opcode)s%(order)s%(suffix)s %(op1)s, %(op2)s, %(op3)s" % (d)
    
def gen(opcodes, combos, optypes, orders):
    for opcode in opcodes:
        for (suffix,width) in combos:
            for order in orders:
                for optype in optypes:
                    emit(opcode,suffix,width,order,optype)
    

if __name__ == '__main__':
    orders = ['132', '231', '213']

    all_combos = [('ss',128),
                  ('sd',128),
                  ('ps',128),
                  ('ps',256),
                  ('pd',128),
                  ('pd',256) ]
    packed_combos = [ ('ps',128),
                      ('ps',256),
                      ('pd',128),
                      ('pd',256) ]
    
    opcodes1 = ['fmadd', 'fmsub', 'fnmadd', 'fnmsub']
    opcodes2 = ['fmaddsub', 'fmsubadd']
    
    optypes = ['rrr','rrm']
    
    print "[bits 64]"
    gen(opcodes1,    all_combos,optypes, orders)
    gen(opcodes2, packed_combos,optypes, orders)
