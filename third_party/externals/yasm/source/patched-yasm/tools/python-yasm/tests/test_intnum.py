from tests import TestCase, add
from yasm import IntNum

class TIntNum(TestCase):
    legal_values = [
        0, 1, -1, 2, -2, 17, -17,
        2**31-1, -2**31, 2**31, 2**32-1, -2**32,
        2**63-1, -2**63-1, 2**63, 2**64, -2**64,
        2**127-1, -2**127
    ]
    overflow_values = [
        2**127, -2**127-1
    ]

    def test_to_from(self):
        for i in self.legal_values:
            self.assertEquals(i, int(IntNum(i)))
            self.assertEquals(i, long(IntNum(i)))

    def test_overflow(self):
        for i in self.overflow_values:
            self.assertRaises(OverflowError, IntNum, i)

    str_values = [
        "0", "00000", "1234", "87654321", "010101010", "FADCBEEF"
    ]
    base_values = [2, 8, 10, 12, 16, None, "nasm", "foo"]

    def test_from_str(self):
        pass

    def test_from_str_base(self):
        pass

    def test_exceptions(self):
        self.assertRaises(ZeroDivisionError, IntNum(1).__div__, 0)

        IntNum(1) / 1 # make sure the above error is cleared

        try: IntNum(1) / 0
        except ZeroDivisionError, err:
            self.assertEquals('divide by zero', str(err))

    def test_xor(self):
        a = IntNum(-234)
        b = IntNum(432)
        c = a ^ b
        self.assertEquals(a, -234)
        self.assertEquals(b, 432)
        self.assertEquals(c, -234 ^ 432)

    def test_ixor(self):
        a = IntNum(-234)
        b = IntNum(432)
        a ^= b; b ^= a; a ^= b
        self.assertEquals(a, 432)
        self.assertEquals(b, -234)

    def test_cmp(self):
        a = IntNum(-1)
        b = IntNum(0)
        c = IntNum(1)
        self.assert_(a < b < c)
        self.assert_(a <= b <= c)
        self.assert_(c >= b >= a)
        self.assert_(c > b > a)
        self.assert_(a != b != c)

    def test_abs(self):
        a = IntNum(-1)
        b = IntNum(0)
        c = IntNum(1)

        self.assertEquals(abs(a), abs(c))
        self.assertEquals(abs(a) - abs(c), abs(b))

add(TIntNum)
