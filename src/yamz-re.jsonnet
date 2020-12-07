// some regex related to yamz.
// fixme: some of these might be good to move up to moo.

local moo = import "moo.jsonnet";

local onemany = function(one, delim=',', space=' ?') {
    // a string with just one thing
    singular: '^' + one + '$',
    // a string encoding multiple things
    plural: '^(%s)((%s)(%s)(%s)(%s))*$' % [
        one, space, delim, space, one],
};
local onetwo = function(one, two, delim='=', space=' ?')
'(%s)(%s)(%s)(%s)(%s)' % [one, space, delim, space, two];
    
local re = {

    qkey: moo.re.ident,
    qval: '[^&#]*',
    qone: '(%s)=(%s)' % [self.qkey, self.qval],
    query: '\\?(%s)(&(%s))*' % [self.qone, self.qone],

    // fully-qualified 
    concrete: {
        ipc: '%s(%s)?' % [moo.re.zmq.ipc.uri, re.query],
        tcp: '%s(%s)?' % [moo.re.zmq.tcp.uri, re.query],
        inproc: 'inproc://([^*?,;&=]{1,255})(%s)?' % re.query,
        uri: '(%s)|(%s)|(%s)' % [self.ipc, self.tcp, self.inproc],
    },

    ephemeral : {
        tcp: 'tcp://((\\*)|(%s)|(%s))((:\\*)|%s)(%s)?' % [moo.re.dnshost, moo.re.ipv4, moo.re.tcpport, re.query],

        hierpath: '/?(((\\*)|(%s))/?)+' % moo.re.hiername,
        ipc: 'ipc://%s(%s)?' % [self.hierpath, re.query],

        inproc: 'inproc://([^?,;&=]{1,255})(%s)?' % re.query,
        uri: '(%s)|(%s)|(%s)' % [self.ipc, self.tcp, self.inproc],
    },

    abstract :  {
        uri: 'yamz://((\\*)|(%s))/(%s)(%s)?' % [re.ephemeral.hierpath, re.ephemeral.hierpath,re.query],
    }
};


local main = re + {

    address: {
        concrete: onemany(re.concrete.uri),
        ephemeral: onemany(re.ephemeral.uri),
        abstract: onemany(re.abstract.uri),
    },

    client: {
        config: {
            binds: '^((%s)|(%s))$' % [re.concrete.uri, re.ephemeral.uri],
            conns: '^((%s)|(%s))$' % [re.concrete.uri, re.abstract.uri],
        }
    },

    portpair: onetwo(self.address.abstract.singular,
                     moo.re.zmq.socket.name, ':'),

    acpair: onetwo(self.portpair, self.address.concrete.plural, '='),
    
    header: onetwo(self.portpair,
                   onemany(self.acpair, ';').plural),

};

main
