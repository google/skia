from tests import TestCase, add
from yasm import SymbolTable, Expression, YasmError

class TSymbolTable(TestCase):
    def setUp(self):
        self.symtab = SymbolTable()

    def test_keys(self):
        self.assertEquals(len(self.symtab.keys()), 0)
        self.symtab.declare("foo", None, 0)
        keys = self.symtab.keys()
        self.assertEquals(len(keys), 1)
        self.assertEquals(keys[0], "foo")

    def test_contains(self):
        self.assert_("foo" not in self.symtab)
        self.symtab.declare("foo", None, 0)
        self.assert_("foo" in self.symtab)

    def test_exception(self):
        expr = Expression('+', 1, 2)
        self.symtab.define_equ("foo", expr, 0)
        self.assertRaises(YasmError, self.symtab.define_equ, "foo", expr, 0)
        self.symtab.define_equ("bar", expr, 0) # cleared
        self.assertRaises(YasmError, self.symtab.define_special, "bar",
                'global')

    def test_iters(self):
        tab = self.symtab
        tab.declare("foo", None, 0)
        tab.declare("bar", None, 0)
        tab.declare("baz", None, 0)

        # while ordering is not known, it must be consistent
        self.assertEquals(list(tab.keys()), list(tab.iterkeys()))
        self.assertEquals(list(tab.values()), list(tab.itervalues()))
        self.assertEquals(list(tab.items()), list(tab.iteritems()))
        self.assertEquals(list(tab.iteritems()), zip(tab.keys(), tab.values()))

add(TSymbolTable)

class TSymbolAttr(TestCase):
    def setUp(self):
        self.symtab = SymbolTable()
        self.declsym = self.symtab.declare("foo", None, 0)

    def test_visibility(self):
        sym = self.symtab.declare("local1", None, 0)
        self.assertEquals(sym.visibility, set())
        sym = self.symtab.declare("local2", '', 0)
        self.assertEquals(sym.visibility, set())
        sym = self.symtab.declare("local3", 'local', 0)
        self.assertEquals(sym.visibility, set())
        sym = self.symtab.declare("global", 'global', 0)
        self.assertEquals(sym.visibility, set(['global']))
        sym = self.symtab.declare("common", 'common', 0)
        self.assertEquals(sym.visibility, set(['common']))
        sym = self.symtab.declare("extern", 'extern', 0)
        self.assertEquals(sym.visibility, set(['extern']))
        sym = self.symtab.declare("dlocal", 'dlocal', 0)
        self.assertEquals(sym.visibility, set(['dlocal']))

        self.assertRaises(ValueError,
                          lambda: self.symtab.declare("extern2", 'foo', 0))
    def test_name(self):
        self.assertEquals(self.declsym.name, "foo")

    def test_equ(self):
        self.assertRaises(AttributeError, lambda: self.declsym.equ)

    def test_label(self):
        self.assertRaises(AttributeError, lambda: self.declsym.label)

    def test_is_special(self):
        self.assertEquals(self.declsym.is_special, False)

    def test_is_curpos(self):
        self.assertEquals(self.declsym.is_curpos, False)

add(TSymbolAttr)
