network jackson
{
    @display("bgb=1116,471");
    types:
        channel C extends ned.DatarateChannel
        {
        }
    submodules:

        I_injecotr: injector {
            @display("p=85,50");
            Dst_Addr = 005;
            delayTime = 10s;
            pktSize = 1bit;
            n_Pkt = 1000;
        }
        I_fisico0: simple_fisico {
            @display("p=85,211");
        }
        I_router: router {
            @display("p=85,128");
            Config_File = xmldoc("jackson2.xml"); 
            Addr = 0;
        }
        
        r1_fisico0: simple_fisico {
            @display("p=172,211");
        }
        r1_router: router {
            @display("p=252,211");
            Config_File = xmldoc("jackson1.xml");
            Addr = 1;
        }
        r1_fisico1: simple_fisico {
            @display("p=252,128");
        }
        r1_fisico2: simple_fisico {
            @display("p=252,301");
        }
        
        r2_fisico0: simple_fisico {
            @display("p=391,128");
        }
        r2_router: router {
            @display("p=391,50");
            Config_File = xmldoc("jackson2.xml");
            Addr = 2;
        }
        r2_fisico1: simple_fisico {
            @display("p=503,50");
        }
        
        r3_fisico0: simple_fisico {
            @display("p=391,301");
        }
        r3_router: router {
            @display("p=391,406");
            Config_File = xmldoc("jackson2.xml");
            Addr = 3;
        }
        r3_fisico1: simple_fisico {
            @display("p=503,406");
        }
        
        r4_fisico0: simple_fisico {
            @display("p=674,50");
        }
        r4_router: router {
            @display("p=674,211");
            Config_File = xmldoc("jackson2.xml");
            Addr = 4;
        }
        r4_fisico1: simple_fisico {
            @display("p=798,211");
        }
        r4_fisico2: simple_fisico {
            @display("p=674,406");
        }
        
        r5_fisico0: simple_fisico {
            @display("p=909,211");
        }
        r5_router: router {
            @display("p=909,137");
            Config_File = xmldoc("jackson2.xml");
            Addr = 5;
        }        
        r5_dump: dump {
            @display("p=909,50");
        }
        
    connections:
        I_injecotr.down_out --> I_router.up_in[0];
        I_router.down_out[1] --> I_fisico0.up_in;

        r1_fisico0.up_out --> r1_router.down_in[0];
        r1_router.down_out[1] --> r1_fisico1.up_in;
        r1_router.down_out[2] --> r1_fisico2.up_in;

        r2_fisico0.up_out --> r2_router.down_in[0];
        r2_router.down_out[1] --> r2_fisico1.up_in;

        r3_fisico0.up_out --> r3_router.down_in[0];
        r3_router.down_out[1] --> r3_fisico1.up_in;

        r4_fisico0.up_out --> r4_router.down_in[0];
        r4_router.down_out[1] --> r4_fisico1.up_in;
        r4_fisico2.up_out --> r4_router.down_in[2];

        r5_fisico0.up_out --> r5_router.down_in[0];
        r5_router.up_out[0] --> r5_dump.down_in;
        r5_router.down_out[1] --> r5_fisico0.up_in;
        
        I_fisico0.out --> C --> r1_fisico0.in;
        r1_fisico1.out --> C --> r2_fisico0.in;
        r1_fisico2.out --> C --> r3_fisico0.in;
        r2_fisico1.out --> C --> r4_fisico0.in;
        r3_fisico1.out --> C --> r4_fisico2.in;        
        r4_fisico1.out --> C --> r5_fisico0.in;
}
