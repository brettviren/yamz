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
    
    secs: s.number("Secs", "i8"),
    nsecs: s.number("NanoSecs", "i4"),    
    sns: s.record("UnixTime", [
        s.field("s", self.secs),
        s.field("ns", self.nsecs),
    ], doc="Seconds since Unix epoch and nanoseconds since second"),

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


    // Client port config
    client_port: s.record("ClientPort", [
        s.field("portid", self.ident,
                doc="Identify port to network and application"),
        s.field("ztype", self.socktype,
                doc="The ZeroMQ socket type"),
        s.field("idparms", self.ident_parms,
                default=[],
                doc="Any port-level identity parameters"),
        s.field("idpatts", self.ident_patts,
                default=[],
                doc="Any port-level identity patterns"),
        s.field("binds", self.epheaddrs,
                default=[],
                doc="Ephemeral bind addresses"),
        s.field("conns", self.abstaddrs,
                default=[],
                doc="Abstract connect addresses"),
    ], doc="Describe one client port"),
    client_ports: s.sequence("ClientPorts", self.client_port),

    // Client config.  This is also the request to the server, though
    // server ignores "servers".
    client_cfg: s.record("ClientConfig", [
        s.field("clientid", self.ident,
                doc="The name by which the client is known in the node"),
        s.field("servers", self.concaddrs,
                default=[],
                doc="The server addresses to which the client shall connect"),
        s.field("idparms", self.ident_parms,
                default=[],
                doc="Any client-level identity parameters"),
        s.field("idpatts", self.ident_patts,
                default=[],
                doc="Any client-level identity patterns"),
        s.field("ports", self.client_ports,
                doc="Describe ports the client shall provide to application")
    ], doc="A yamz client configuration object"),
    client_cfgs: s.sequence("ClientConfigs", self.client_cfg),

    // Server config
    server_cfg: s.record("ServerConfig", [
        s.field("nodeid", self.ident,
                doc="The name by which this yamz node is known on the network"),
        s.field("addresses", self.concaddrs, default=["inproc://yamz"],
                doc="The addresses to which the server shall bind"),
        s.field("portnum", self.portnum, default=5670,
                doc="The IP port number on which Zyre operates"),
        s.field("idparms", self.ident_parms,
                default=[],
                doc="Any node-level identity parameters"),
        s.field("idpatts", self.ident_patts,
                default=[],
                doc="Any node-level identity patterns"),
        s.field("expected", self.idents,
                default=[],
                doc="The set of client IDs we expect to connect"),
    ], doc="A yamz server configuration object"),


    // The server API has a synchronous protocol with the actor with
    // these message types.
    apicmds: s.enum("ApiCommands", symbols=["terminate","online","offline"]),
    apicmd: s.record("ApiCommand", [
        s.field("command", self.apicmds),
    ], doc="A command from server API to actor"),
    apirpl: s.enum("ApiReply", symbols=["fail","okay"]),

    // The server actor has an asynchronous protocol with clients.
    // maybe:  add "online" and "offline"
    cliact: s.enum("ClientAction", symbols=[
        "connect", "disconnect", "terminate","timeout"]),
    clirpl: s.record("ClientReply", [
        s.field("action", self.cliact,
                doc="What action the client should take on port"),
        s.field("portid", self.ident,
                default="",
                doc="Identify client port"),
        s.field("address", self.concaddr,
                default="",
                doc="The addresses to act on with to the port"),
    ], doc="A reply sent to a client"),
    clirpls: s.sequence("ClientReplies", self.clirpl),


    // Zyre ENTER events carry a header "YAMZ" with a value in this
    // structure.  The nodeid held by the zyre node nick name
    yamz_peer: s.record("YamzPeer", [
        s.field("idparms", self.ident_parms),
        s.field("clients", self.yamz_clients),
    ], doc="The discovered information about a peer"),
    yamz_client: s.record("YamzClient", [
        s.field("clientid", self.ident),
        s.field("idparms", self.ident_parms),
        s.field("ports", self.yamz_ports),
    ], doc="The discovered information about a peer client"),
    yamz_clients: s.sequence("YamzClients", self.yamz_client),
    yamz_port: s.record("YamzPort", [
        s.field("portid", self.ident),
        s.field("ztype", self.socktype),
        s.field("idparms", self.ident_parms),
        s.field("addresses", self.concaddrs),
    ], doc="The discovered information about a peer client port"),
    yamz_ports: s.sequence("YamzPorts", self.yamz_port),
        

    // for test_yamz_cluster
    test_treq: s.record("TestTimeRequest", [
        s.field("reqtime", self.sns, doc="Time of request"),
    ], doc="Make a request for a remote time"),
    test_trep: s.record("TestTimeReply", [
        s.field("reptime", self.sns, doc="Time of reply"),
        s.field("reqtime", self.sns, doc="Echo back request time"),
    ], doc="Reply to a time request"),
    test_job: s.record("TestJobCfg", [
        s.field("server", self.server_cfg),
        s.field("clients", self.client_cfgs)
    ], doc="Overall configuration object for test_yamz_cluster")

};
moo.oschema.sort_select(yamz)

