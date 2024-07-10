// code originally from itsembedded.com and modified by thuvasooriya
#include "obj_dir/Valu.h"
#include "obj_dir/Valu___024unit.h"
#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

#define MAX_SIM_TIME 300
#define VERIF_START_TIME 7
vluint64_t sim_time = 0;
vluint64_t posedge_count = 0;

void dut_reset(Valu *dut, vluint64_t &sim_time) {
  dut->rst = 0;
  if (sim_time > 3 && sim_time < 6) {
    dut->rst = 1;
    dut->a_in = 0;
    dut->b_in = 0;
    dut->op_in = 0;
    dut->in_valid = 0;
  }
}

void check_out_valid(Valu *dut, vluint64_t &sim_time) {
  static unsigned char in_valid = 0;
  static unsigned char in_valid_d = 0;
  static unsigned char out_valid_exp = 0;

  if (sim_time >= VERIF_START_TIME) {
    out_valid_exp = in_valid_d;
    in_valid_d = in_valid;
    in_valid = dut->in_valid;
    if (out_valid_exp != dut->out_valid) {
      std::cout << "err: out_valid mismatch, "
                << "exp: " << (int)out_valid_exp
                << "recv: " << (int)(dut->out_valid) << "simtime: " << sim_time
                << std::endl;
    }
  }
}

void set_rnd_out_valid(Valu *dut, vluint64_t &sim_time) {
  if (sim_time >= VERIF_START_TIME) {
    dut->in_valid = rand() % 2;
  }
}

int main(int argc, char **argv, char **env) {
  srand(time(NULL));
  Verilated::commandArgs(argc, argv);
  Valu *dut = new Valu;

  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC;
  dut->trace(m_trace, 5);
  m_trace->open("waveform.vcd");

  while (sim_time < MAX_SIM_TIME) {
    dut_reset(dut, sim_time);

    dut->clk ^= 1;
    dut->eval();

    if (dut->clk == 1) {
      dut->in_valid = 0;
      posedge_count++;
      // std::cout << "posedge_count: " << posedge_count << std::endl;
      // if (posedge_count ==5){
      //     // std::cout << "setting in_valid" << std::endl;
      //     dut->in_valid = 1;
      // }
      // std::cout << "in_valid: " << dut->in_valid << std::endl;
      // std::cout << "out_valid: " << dut->out_valid << std::endl;
      // if (posedge_count ==7){
      //     if (dut->out_valid != 1){
      //         std::cout << "Error: out_void is not 1" << std::endl;
      //     }
      // }
      // set_rnd_out_valid(dut, sim_time);
      switch (posedge_count) {
      case 10:
        dut->in_valid = 1;
        dut->a_in = 5;
        dut->b_in = 3;
        dut->op_in = Valu___024unit::operation_t::add;
        break;
      case 12:
        if (dut->out != 8)
          std::cout << "addition failed @ " << sim_time << std::endl;
        break;
      case 20:
        dut->in_valid = 1;
        dut->a_in = 5;
        dut->b_in = 3;
        dut->op_in = Valu___024unit::operation_t::sub;
        break;
      case 22:
        if (dut->out != 2)
          std::cout << "subtraction failed @ " << sim_time << std::endl;
        break;
      }
      check_out_valid(dut, sim_time);
    }
    m_trace->dump(sim_time);
    sim_time++;
  }

  m_trace->close();
  delete dut;
  exit(EXIT_SUCCESS);
}
