class SymbolTable:
    """ Symbol table """

    def __init__(self):
        self.symbols = [('global', {})]  # stack of (function scope name, dict)


    def popScope(self):
        assert len(self.symbols) > 1
        del self.symbols[-1]


    def pushScope(self, name):
        self.symbols.append((name, {}))


    def append(self, symname, obj):
        d = self.symbols[-1][1]
        d[symname] = obj


    def find(self, synname):
        """
        Find a symbol with synname.
        If a symbol was not found, return None.
        """

        for i in range(len(self.symbols)):
            d = self.symbols[i][1]
            if d.has_key(synname):
                return d[synname]

        return None


    def lookup(self, synname):
        """
        Find a symbol with synname.
        If a symbol was not found, raise a exeption.
        """

        for i in range(len(self.symbols)):
            d = self.symbols[i][1]
            if d.has_key(synname):
                return d[synname]

        raise Exception("Undefine symbol: ", synname)
