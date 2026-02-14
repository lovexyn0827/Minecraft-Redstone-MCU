module rs_ecc #(
    parameter   N = 12,             // Message length
    parameter   K = 12,             // ECC length
    parameter   PP = 8'b00101101,   // Primitive
    parameter   GP = {
        8'd1, 8'd242, 8'd100, 8'd178, 8'd97, 8'd213, 
        8'd142, 8'd42, 8'd61, 8'd91, 8'd158, 8'd153, 
        8'd41
    },                              // Generator polynomial
    parameter   ADDR_LEN = 4
) ( 
    input   wire                        clk, rst, 
    input   wire    [7:0]               sdat_in, 
    input   wire                        sdat_in_rdy, 
    input   wire    [ADDR_LEN - 1:0]    ecc_addr,      // ecc_out is unspecified when out of bound
    output  wire                        rdy, 
    output  wire                        req_word, 
    output  wire    [7:0]               ecc_out
);

reg     [7:0]   b       [K - 1:0];
wire    [7:0]   b_left  [K - 1:0];
reg     [7:0]   z       [K - 1:0];

assign b_left[0] = 8'b0;
assign ecc_out = b[K - ecc_addr - 1];

reg     [2:0]           mul_counter;
reg     [ADDR_LEN:0]    word_counter;

assign rdy = word_counter >= N;
assign req_word = ~rdy & mul_counter == 3'b0;

reg     [7:0]   x;
wire    [7:0]   next_x;

assign next_x = sdat_in ^ b[K - 1];

generate
    genvar i;
    for (i = 1; i < K; i = i + 1) begin: b_left_gen
        assign b_left[i] = b[i - 1];
    end
endgenerate

always @(posedge clk) begin
    if (rst) begin
        mul_counter <= 3'b0;
        x <= 8'b0;
        word_counter <= 5'b0;for (integer i = 0; i < K; i = i + 1) begin
            b[i] <= 8'b0;
            z[i] <= 8'b0;
        end
    end else if (~rdy) begin
        mul_counter <= ((mul_counter == 3'b000) & ~sdat_in_rdy) ? mul_counter : mul_counter - 3'b1;
        if ((mul_counter == 3'b000) & sdat_in_rdy) begin
            word_counter <= word_counter + 1;
            x <= next_x << 1;
            for (integer i = 0; i < K; i = i + 1) begin
                b[i] <= b_left[i] ^ z[i];
                z[i] <= { 8 { next_x[7] } } & GP[i];
            end
        end else if (mul_counter != 3'b000) begin
            x <= x << 1;
            for (integer i = 0; i < K; i = i + 1) begin
                z[i] <= (z[i] << 1) ^ ({ 8 { z[i][7] } } & PP) ^ ({ 8 { x[7] } } & GP[i]);
            end
        end
    end
end

endmodule