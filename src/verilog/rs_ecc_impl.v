module rs_ecc_impl # (
    parameter   N = 16,             // Message length
    parameter   K = 10,             // ECC length
    parameter   PP = 8'b00011101,   // Primitive
    parameter   GP = {
        8'd1, 8'd216, 8'd194, 8'd159, 8'd111, 8'd199, 8'd94, 8'd95, 8'd113, 8'd157, 8'd193
    },                              // Generator polynomial
    parameter   ADDR_LEN = 4
) ( 
    input   wire                        clk, rst, 
    input   wire    [7:0]               sdat_in, 
    input   wire                        sdat_in_rdy, 
    input   wire    [ADDR_LEN - 1:0]    ecc_addr,      // ecc_out is unspecified when out of bound
    output  wire                        rdy, 
    output  wire    [7:0]               ecc_out
);

wire [7:0] gp[K:0];

generate
    genvar k;
    for (k = 0; k <= K; k = k + 1) begin: gen_gp_val
        assign gp[k] = GP[k * 8 + 7:k * 8];
    end 
endgenerate

reg     [7:0]   b       [K - 1:0];
wire    [7:0]   b_left  [K - 1:0];
reg     [7:0]   z       [K - 1:0];

assign b_left[0] = 8'b0;
assign ecc_out = b[K - ecc_addr - 1];

reg     [2:0]           mul_counter;
reg     [ADDR_LEN:0]    word_counter;

assign rdy = word_counter > N;

reg     [7:0]   x;
wire    [7:0]   next_x;

assign next_x = sdat_in ^ b_left[K - 1] ^ z[K - 1];

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
        word_counter <= 5'b0;for (integer j = 0; j < K; j = j + 1) begin
            b[j] <= 8'b0;
            z[j] <= 8'b0;
        end
    end else if (~rdy) begin
        mul_counter <= ((mul_counter == 3'b000) & ~sdat_in_rdy) ? mul_counter : mul_counter - 3'b1;
        if ((mul_counter == 3'b000) & sdat_in_rdy) begin
            word_counter <= word_counter + 1;
            x <= next_x << 1;
            for (integer j = 0; j < K; j = j + 1) begin
                b[j] <= b_left[j] ^ z[j];
                z[j] <= { 8 { next_x[7] } } & gp[j];
            end
        end else if (mul_counter != 3'b000) begin
            x <= x << 1;
            for (integer j = 0; j < K; j = j + 1) begin
                z[j] <= (z[j] << 1) ^ ({ 8 { z[j][7] } } & PP) ^ ({ 8 { x[7] } } & gp[j]);
            end
        end
    end
end

endmodule
