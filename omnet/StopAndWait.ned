//
// Created by: Ainhoa Sayans
//


network StopAndWait
{
    @display("bgb=826,265");
    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 1bps;
            per = 0.1;
        }
    submodules:
        injector: injector {
            @display("p=45,33");
            Dst_Addr = 1;
            delayTime = exponential(30s);
        }
        sw: free_sw {
            @display("p=45,107");
            Addr = 0;
        }
        fisico: fisico {
            @display("p=45,190");
            Queue_Length = 20;
        }
        fisico1: fisico {
            @display("p=539,190");
        }
        sw1: free_sw {
            @display("p=539,107");
            Addr = 1;
        }
        dump: dump {
            @display("p=539,33");
        }
    connections:
        injector.down_out --> sw.up_in;

        sw.down_out --> fisico.up_in;
        fisico.up_out --> sw.down_in;

        sw1.down_out --> fisico1.up_in;
        fisico1.up_out --> sw1.down_in;

        fisico.out --> C --> fisico1.in;
        fisico1.out --> C --> fisico.in;

        sw1.up_out --> dump.down_in;

}
