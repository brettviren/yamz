#!/usr/bin/env python3
'''
Generate config for test_yamz_cluster process.

Usage: python3 test_yamz_cluster.py NAME NUMBER > NAME.json

  NAME is the yamz/zyre node name
  NUMBER is how may "client" components to use in the process

To validate, assuming one is in package top level:

  $ moo -M src validate -s test/test-yamz-cluster-valid.jsonnet NAME.json
  null

'''



import os
import sys
import moo
import json

tstdir = os.path.dirname(os.path.realpath(__file__))
topdir = os.path.dirname(tstdir)
srcdir = os.path.join(topdir, "src")

for one in moo.io.load("yamz-schema.jsonnet", [srcdir]):
    moo.otypes.make_type(**one)
import yamz

nodename = sys.argv[1]
client_names = ["comp%02d" % n for n in range(int(sys.argv[2]))]

sc = yamz.ServerConfig(nodeid=sys.argv[1],
                       expected=client_names)
ccs = list()
for cname in client_names:
    cc = yamz.ClientConfig(clientid=cname)
    cc.ports = yamz.ClientPorts([
        yamz.ClientPort(portid="askme", ztype="SERVER",
                        binds=["tcp://*:*"]),
        yamz.ClientPort(portid="askyou", ztype="CLIENT",
                        conns=["yamz://*/*/askme"]),
    ])
    ccs.append(cc)
jc = yamz.TestJobCfg(server=sc, clients=yamz.ClientConfigs(ccs))
pod = jc.pod()
print(json.dumps(pod, indent=4))
