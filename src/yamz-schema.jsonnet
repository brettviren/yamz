local moo = import "moo.jsonnet";
local s = moo.oschema.schema("yamz");

local re = import "yamz-re.jsonnet";

local yamz = {

    // Identity

    ident: s.string("Ident", pattern=moo.re.ident_only,
                    doc="An identifier or name"),
    idents: s.sequence("Idents", self.ident),

    key: s.string("Key", re.qkey, doc="A key name"),
    pval: s.string("ParameterValue", re.qval, doc="An identity parameter value"),
    patt: s.string("PatternValue", re.qval, doc="An identity pattern value"),
    
    ident_parm: s.record("IdentityParameter", [
        s.field("key", self.key, doc="The key name of the parameter"),
        s.field("val", self.pval, doc="The value of the parameter"),
    ], doc="An identity parameter"),
    ident_parms: s.sequence("IdentityParameters", self.ident_parm),
    
    ident_patt: s.record("IdentityPattern", [
        s.field("key", self.key, doc="The key name of the pattern"),
        s.field("val", self.patt, doc="The pattern string"),
    ], doc="An identity pattern"),
    ident_patts: s.sequence("IdentityPatterns", self.ident_patt),    

    // misc 

    hpath: s.string("Path", pattern=re.address.abstract.plural,
                    doc="A hierarchy path"),

    socktype: s.enum("SockType", symbols=moo.re.zmq.socket.name_list,
                     doc="Enumerate ZeroMQ socket names in canoncial order"),

    portnum: s.number("PortNum", "i4", doc="A IP port number"),

    // URI strings

    concaddr: s.string("ConcreteAddress",
                       pattern=re.address.concrete.singular,
                       doc="Concrete address"),
    concaddrs: s.sequence("ConcreteAddresses", self.concaddr),

    epheaddr: s.string("EphemeralAddress",
                       pattern=re.address.ephemeral.singular,
                       doc="Ephemeral Address"),
    epheaddrs: s.sequence("EphemeralAddresses", self.epheaddr),

    abstaddr: s.string("AbstractAddress",
                       pattern=re.address.abstract.singular,
                       doc="Abstract Address"),
    abstaddrs: s.sequence("AbstractAddresses", self.abstaddr),


    // Server config
    server_cfg: s.record("ServerConfig", [
        s.field("nodeid", self.ident,
                doc="The name by which this yamz node is known on the network"),
        s.field("portnum", self.portnum, default=5670,
                doc="The IP port number on which Zyre operates"),
        s.field("addresses", self.concaddrs, default=["inproc://yamz"],
                doc="The addresses to which the server shall bind"),
        s.field("idparms", self.ident_parms,
                doc="Any node-level identity parameters"),
        s.field("idpatts", self.ident_patts,
                doc="Any node-level identity patterns"),
        s.field("expected", self.idents,
                doc="A set of peer nodes to expect to discover"),
    ], doc="A yamz server configuration object"),


    // Client config

    client_port: s.record("ClientPort", [
        s.field("portid", self.ident,
                doc="Identify port to network and application"),
        s.field("ztype", self.socktype,
                doc="The ZeroMQ socket type"),
        s.field("idparms", self.ident_parms,
                doc="Any port-level identity parameters"),
        s.field("idpatts", self.ident_patts,
                doc="Any port-level identity patterns"),
        s.field("binds", self.epheaddrs,
                doc="Ephemeral bind addresses"),
        s.field("conns", self.abstaddrs,
                doc="Abstract connect addresses"),
    ], doc="Describe one client port"),
    client_ports: s.sequence("ClientPorts", self.client_port),

    // Client config
    client_cfg: s.record("ClientConfig", [
        s.field("clientid", self.ident,
                doc="The name by which the client is known in the node"),
        s.field("servers", self.concaddrs, default=["inproc://yamz"],
                doc="The server addresses to which the client shall connect"),
        s.field("idparms", self.ident_parms,
                doc="Any client-level identity parameters"),
        s.field("idpatts", self.ident_patts,
                doc="Any client-level identity patterns"),
        s.field("ports", self.client_ports,
                doc="Describe ports the client shall provide to application")
    ], doc="A yamz client configuration object"),


    // Client/server protocol messages use ClientConfig


};
moo.oschema.sort_select(yamz)

