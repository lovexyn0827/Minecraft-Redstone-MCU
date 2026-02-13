module alu (
    input   wire    [7:0]   a, b, 
    input   wire    [3:0]   func, 
    output  wire    [7:0]   z
);

wire [7:0] results [15:0];

assign results[0] = a + b;
assign results[1] = a - b;
assign results[2] = a & b;
assign results[3] = a | b;
assign results[4] = a ^ b;
assign results[5] = a >> b;
assign results[6] = a << b;
assign results[7] = a >>> b;
assign results[8] = a | (8'b1 << b[2:0]);
assign results[9] = a & ~(8'b1 << b[2:0]);
assign results[10] = a;
assign results[11] = a;
assign results[12] = a == b ? 1 : 0;
assign results[13] = a != b ? 1 : 0;
assign results[14] = a > b ? 1 : 0;
assign results[15] = a < b ? 1 : 0;

assign z = results[func];

endmodule
