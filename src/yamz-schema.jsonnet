local moo = import "moo.jsonnet";
local s = moo.oschema.schema("yamz");

local re = import "yamz-re.jsonnet";

local yamz = {

    ident: s.string("Ident", pattern=moo.re.ident_only,
                    doc="An unique identifier"),

    hpath: s.string("Path", pattern=re.address.abstract.plural,
                    doc="A hierarchy path"),

    zhead: s.string("Header", pattern=re.header,
                    doc="YAMZ zyre header value"),

    socktype: s.enum("SockType", symbols=moo.re.zmq.socket.name_list,
                     doc="Enumerate ZeroMQ socket names in canoncial order"),

    concaddr: s.string("ConcreteAddress",
                       pattern=re.address.concrete.singular,
                       doc="Concrete address"),



    concaddrs: s.sequence("ConcreteAddresses", self.concaddr),

    // Taxonomy:
    //
    // - node :: nick name of a zyre peer, typically associated with
    //           one process, unique to network
    // 
    // - comp :: name of a component instance in a node which has
    //           sockets, unique to node
    //
    // - port :: name of a socket, unique to component.  A port has a
    //           socket type and a number of addresses which may be
    //           concrete or abstract.
    
    // A concrete address simply means it is fully specified.  No
    // wildcards ("*") and in a ZeroMQ canonical URI scheme.  These
    // are used by client to tell server what are the client has
    // bind()'ed.  They are published to discovery using the server's
    // node name.  These are also returned to the client holding
    // resolved addresses to connect().


    concport: s.record("ConcretePort", [
        s.field("portid", self.ident,
                doc="Identify a port of the client"),
        s.field("ztype", self.socktype,
                doc="The ZeroMQ socket type"),
        s.field("concs", self.concaddrs,
                doc="Concrete addresses associated with port"),
    ], doc="An association of a port and its concrete addresses"),
    concports: s.sequence("ConcretePorts", self.concport),

    // Abstract address means it is given in terms of a Zyre node aka
    // peer name, a component name and a port name associated with a
    // socket in the network peer.

    abstaddr: s.record("AbstractAddress", [
        s.field("node", self.ident,
                doc="Identify a node"),
        s.field("comp", self.ident,
                doc="Identify a node's component"),
        s.field("port", self.ident,
                doc="Identify a component's port"),
    ], doc="Abstractly identify a socket address"),
    abstaddrs: s.sequence("AbstractAddresses", self.abstaddr),

    abstport: s.record("AbstractPort", [
        s.field("port", self.ident,
                doc="Name of the clients port that wants to connect"),
        s.field("addrs", self.abstaddrs,
                doc="Abstract addresses for server to match"),
    ], doc="All addresses to resolve for one client port"),
    abstports: s.sequence("AbstractPorts", self.abstport),

    request: s.record("Request", [
        s.field("comp", self.ident,
                doc="Uniquely identify the client"),
        s.field("binds", self.concports,
                doc="Concrete bind ports made by client"),
        s.field("conns", self.abstports,
                doc="Abstract connect ports to be resolved by server"),
    ], doc="The structure of a client request made to a server"),

    reply: s.record("Reply", [
        s.field("comp", self.ident,
                doc="The requesting client component identity"),
        s.field("conns", self.concports,
                doc="A set of concrete port addresses client may connect"),
    ], doc="Request reply from server to client"),


         
};
moo.oschema.sort_select(yamz)

