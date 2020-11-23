// some regex related to yamz.
// fixme: some of these might be good to move up to moo.

local moo = import "moo.jsonnet";

local onemany = function(one, delim, space=' ?') {
    singular: '^' + one + '$',
    plural: '^(%s)((%s)(%s)(%s)(%s))*$' % [
        one, space, delim, space, one],
};
local onetwo = function(one, two, delim='=', space=' ?')
'(%s)(%s)(%s)(%s)(%s)' % [one, space, delim, space, two];
    

{
    zmq: {
        addrs: std.join('|',[moo.re.tcp, moo.re.ipc, moo.re.inproc]),
    },

    address: {
        concrete: onemany($.zmq.addrs, ',', ' ?'),
        abstract: onemany(moo.re.ident, "/", ''),
    },

    portpair: onetwo(self.address.abstract.singular,
                     self.zmq.socktype, ':'),

    acpair: onetwo(self.portpair, self.address.concrete.plural, '='),

    header: onetwo(self.portpair,
                   onemany(self.acpair, ';').plural),
}
