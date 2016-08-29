//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//



network Pruebas
{
    @display("bgb=810,360");
    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 1bps;
            per = 0;
            delay = 1s;
        }
    submodules:

        injector: injector {
            @display("p=145,113");
        }
        simple_fisico: simple_fisico {
            @display("p=145,207");
        }
        dump: dump {
            @display("p=352,113");
        }
        simple_fisico1: simple_fisico {
            @display("p=352,207");
        }
    connections:
        injector.down_out --> simple_fisico.up_in;
        simple_fisico1.up_out --> dump.down_in;
        simple_fisico.out --> C --> simple_fisico1.in;
}
