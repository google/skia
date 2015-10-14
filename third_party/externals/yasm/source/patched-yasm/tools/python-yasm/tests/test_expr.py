from tests import TestCase, add
from yasm import Expression
import operator

class TExpression(TestCase):
    def test_create(self):
        e1 = Expression(operator.add, 1, 2)
        e2 = Expression('+', 1, 2)
        
        self.assertEquals(e1.get_intnum(), e1.get_intnum())

    def test_extract(self):
        e1 = Expression('/', 15, 5)
        self.assertEquals(e1.get_intnum(), 3)
        self.assertRaises(ValueError, e1.extract_segoff)
        self.assertRaises(ValueError, e1.extract_wrt)

add(TExpression)
