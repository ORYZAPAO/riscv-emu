// --------------------------------------------------------------------------------
// RISC-V rv32ui Instraction Simple Emurator
//
//  Written by ORYZA
// --------------------------------------------------------------------------------

// Memo
//   mret
//   CSR(0xc00) = cycle

#include<iostream>
#include<fstream>
#include<cassert>
#include <array>
#include<boost/format.hpp>

//using namespace std;

//----------------------------------------------------------------------
//----------------------------------------------------------------------
namespace rvemu
{
  using uint = unsigned int;

  using u32 = unsigned int;
  using u16 = unsigned short;
  using i32 = int;
  using i16 = short;
  
  using REGNO = int; 
  using IMM   = u32;
  using ADDR  = u32;

  ADDR BASE_ADDR     = 0x80000000  >> 2; // Base Address を 32bit単位にする
  ADDR BASE_ADDR_MSK = 0x1FFFFFFF;

 // ------------------------------------------------------------
 // -- RAM Memory
 // ------------------------------------------------------------
  template<class T, int size>
  struct MEMORY : public std::array<T, size> {

    T& operator [](ADDR addr){
      addr &= BASE_ADDR_MSK;

      //if( addr == (0x26364>>2) ) {
      //  std::cout <<"STOP 0x00026364 0x" << hex << std::array<T,size>::at(addr) << endl;
      //}
      return std::array<T,size>::operator [](addr);
    }

    T& at(ADDR addr){
      addr &= BASE_ADDR_MSK;
      return std::array<T,size>::at(addr);
    };  
  };
  
  //std::array<u32, 0x10000> MEM;
  MEMORY<u32, 0x10000> MEM;
  


// ------------------------------------------------------------
// -- Register File
// ------------------------------------------------------------
  //
  // Chapter 25 RISC-V Assembly Programmer’s Handbook
  //
  // Reg ABI Name Description Saver
  // --------------------------------------------------
  // x0  zero   Hard-wired zero —
  // x1  ra     Return address Caller
  // x2  sp     Stack pointer Callee
  // x3  gp     Global pointer —
  // x4  tp     Thread pointer —
  // x5  t0     Temporary/alternate link register Caller
  // x6  t1     Temporaries Caller
  // x7  t2        :
  //
  // x8  s0/fp  Saved register/frame pointer Callee
  // x9  s1     Saved register Callee
  // x10 a0     Function arguments/return values Caller
  // x11 a1     Function arguments/return values Caller
  // x12 a2     Function arguments Caller
  // x13 a3        :
  // x14 a4        :
  // x15 a5        :
  //
  // x16 a6        :
  // x17 a7        :
  //
  // x18–27 s2–11  Saved registers Callee
  //
  // x28–31 t3–6   Temporaries Caller
  struct RegFile{
    u32    reg[32];  
    u32    pc;
    
    RegFile():pc(0u){
      for(auto &val : reg){
        val = 0;
      }
    }
    
    u32& operator [](REGNO i){
      if( i == 0 ) { // Zero Register
        reg[0] = 0;
      }
      return reg[i];
    }

    void report(){
      std::cout << boost::format("   x0,zero: 0x%08x  x1,ra : 0x%08x  x2,sp : 0x%08x  x3,gp : 0x%08x") %
        reg[0]  % reg[1] % reg[2] % reg[3]  << std::endl;    
      std::cout << boost::format("   x4,tp  : 0x%08x  x5,t0 : 0x%08x  x6,t1 : 0x%08x  x7,t2 : 0x%08x") %
        reg[4] % reg[5] % reg[6] % reg[7] << std::endl;    
      std::cout << boost::format("  x8,s0/fp: 0x%08x  x9,s1 : 0x%08x x10,a0 : 0x%08x x11,a1 : 0x%08x") %
        reg[8]  % reg[9] % reg[10] % reg[11] << std::endl;    
      std::cout << boost::format("   x12,a2 : 0x%08x x13,a3 : 0x%08x x14,a4 : 0x%08x x15,a5 : 0x%08x") %
        reg[12] % reg[13] % reg[14] % reg[15] << std::endl;    
      std::cout << boost::format("   x16,a6 : 0x%08x x17,a7 : 0x%08x x18,s2 : 0x%08x x19,s3 : 0x%08x") %
        reg[16] % reg[17] % reg[18] % reg[19] << std::endl;    
      std::cout << boost::format("   x20,s4 : 0x%08x x21,s5 : 0x%08x x22,s6 : 0x%08x x23,s7 : 0x%08x") %
        reg[20] % reg[21] % reg[22] % reg[23] << std::endl;    
      std::cout << boost::format("   x24,s8 : 0x%08x x25,s9 : 0x%08x x26,s10: 0x%08x x27,s11: 0x%08x") %
        reg[24] % reg[25] % reg[26] % reg[27] << std::endl;    
      std::cout << boost::format("   x28,t3 : 0x%08x x29,t4 : 0x%08x x30,t5 : 0x%08x x31,t6 : 0x%08x") %
        reg[28] % reg[29] % reg[30] % reg[31] << std::endl;    

      std::cout << boost::format("PC : 0x%08X") % this->pc << std::endl;
    }
    
  } reg;


// ------------------------------------------------------------
// -- CSR register
// ------------------------------------------------------------
  enum class CSRNUM : uint{
     mhartid   = 0xF14
    ,medeleg  = 0x302
    ,mideleg  = 0x303
    ,mepc     = 0x341
    ,mie      = 0x304
    ,mstatus  = 0x300
    ,mtvec    = 0x305
    ,stvec    = 0x105
    ,pmpaddr0 = 0x3b0
    ,pmpcfg0  = 0x3a0
    ,satp     = 0x180
    ,mcause   = 0x342

    ,cycle   = 0xc00
    ,mcycle  = 0xb00
    ,minstret = 0xb02
  };

  struct CSRs{
    u32 attr_mhartid  ;
    u32 attr_medeleg  ;
    u32 attr_mideleg  ;
    u32 attr_mepc     ;
    u32 attr_mie      ;
    u32 attr_mstatus  ; 
    u32 attr_mtvec    ;
    u32 attr_stvec    ;
    u32 attr_pmpaddr0 ;
    u32 attr_pmpcfg0  ;
    u32 attr_satp     ;
    u32 attr_mcause   ;

    u32 attr_cycle    ;
    u32 attr_mcycle   ;
    u32 attr_minstret ;
    u32 attr_Others   ;
    
    CSRs():
      attr_mhartid(0u) 
    , attr_medeleg(0u) 
    , attr_mideleg(0u) 
    , attr_mepc(0u)    
    , attr_mie(0u)     
    , attr_mstatus(0u)  
    , attr_mtvec(0u)   
    , attr_stvec(0u)   
    , attr_pmpaddr0(0u) 
    , attr_pmpcfg0(0u) 
    , attr_satp(0u)   
    , attr_mcause(0u)
    , attr_cycle(0u) 
    , attr_mcycle(0u)   
    , attr_minstret(0u) 
    , attr_Others(0u) 
    {}
    
    u32& operator [](uint num){      
      switch(num){
      case static_cast<uint>(CSRNUM::mhartid ) : return attr_mhartid  ;
      case static_cast<uint>(CSRNUM::medeleg ) : return attr_medeleg  ;
      case static_cast<uint>(CSRNUM::mideleg ) : return attr_mideleg  ;
      case static_cast<uint>(CSRNUM::mepc    ) : return attr_mepc     ;
      case static_cast<uint>(CSRNUM::mie     ) : return attr_mie      ;
      case static_cast<uint>(CSRNUM::mstatus ) : return attr_mstatus  ;
      case static_cast<uint>(CSRNUM::mtvec   ) : return attr_mtvec    ;
      case static_cast<uint>(CSRNUM::stvec   ) : return attr_stvec    ;
      case static_cast<uint>(CSRNUM::pmpaddr0) : return attr_pmpaddr0 ;
      case static_cast<uint>(CSRNUM::pmpcfg0 ) : return attr_pmpcfg0  ;
      case static_cast<uint>(CSRNUM::satp    ) : return attr_satp     ;
      case static_cast<uint>(CSRNUM::mcause  ) : return attr_mcause   ;
      case static_cast<uint>(CSRNUM::cycle   ) : return attr_cycle    ;
      case static_cast<uint>(CSRNUM::mcycle  ) :{ ++attr_mcycle; return attr_mcycle; }
      case static_cast<uint>(CSRNUM::minstret) : return attr_minstret ;

      default :
        std::cout << boost::format("NOT SUPPORT CSR Number(%03x)") % static_cast<int>(num) << std::endl;
        return attr_Others;
      }
    }


    void report(){
      std::cout << boost::format("[mhartid (0x%03x)] %08x") % static_cast<uint>(CSRNUM::mhartid ) % attr_mhartid  << std::endl;
      std::cout << boost::format("[medeleg (0x%03x)] %08x") % static_cast<uint>(CSRNUM::medeleg ) % attr_medeleg  << std::endl;
      std::cout << boost::format("[mideleg (0x%03x)] %08x") % static_cast<uint>(CSRNUM::mideleg ) % attr_mideleg  << std::endl;
      std::cout << boost::format("[mepc    (0x%03x)] %08x") % static_cast<uint>(CSRNUM::mepc    ) % attr_mepc     << std::endl;
      std::cout << boost::format("[mie     (0x%03x)] %08x") % static_cast<uint>(CSRNUM::mie     ) % attr_mie      << std::endl;
      std::cout << boost::format("[mstatus (0x%03x)] %08x") % static_cast<uint>(CSRNUM::mstatus ) % attr_mstatus  << std::endl;
      std::cout << boost::format("[mtvec   (0x%03x)] %08x") % static_cast<uint>(CSRNUM::mtvec   ) % attr_mtvec    << std::endl;
      std::cout << boost::format("[stvec   (0x%03x)] %08x") % static_cast<uint>(CSRNUM::stvec   ) % attr_stvec    << std::endl;
      std::cout << boost::format("[pmpaddr0(0x%03x)] %08x") % static_cast<uint>(CSRNUM::pmpaddr0) % attr_pmpaddr0 << std::endl;
      std::cout << boost::format("[pmpcfg0 (0x%03x)] %08x") % static_cast<uint>(CSRNUM::pmpcfg0 ) % attr_pmpcfg0  << std::endl;
      std::cout << boost::format("[satp    (0x%03x)] %08x") % static_cast<uint>(CSRNUM::satp    ) % attr_satp     << std::endl;
      std::cout << boost::format("[mcause  (0x%03x)] %08x") % static_cast<uint>(CSRNUM::mcause  ) % attr_mcause   << std::endl;
      std::cout << boost::format("[cycle   (0x%03x)] %08x") % static_cast<uint>(CSRNUM::cycle   ) % attr_cycle    << std::endl;
      std::cout << boost::format("[mcycle  (0x%03x)] %08x") % static_cast<uint>(CSRNUM::mcycle  ) % attr_mcycle   << std::endl;
    }
    
  } CSRs;
  

// ------------------------------------------------------------
// -- Utility  
// ------------------------------------------------------------

  // Unsigned Number to Signed Number
  //
  /*constexpr clang++ NG*/ i32 Int2Sign(unsigned int d32){
    return *reinterpret_cast<i32*>(&d32);
  }
  
  // Generate Bit Mask Pattern
  //
  u32 msk32bit(const int end_bit, const int st_bit){
    assert( (st_bit  >= 0) && (st_bit  <= 31) );
    assert( (end_bit >= 0) && (end_bit <= 31) );

    u32 msk =0x0u;
    for(int i=st_bit; i<=end_bit; i++) 
      msk |= (0x1u) << i;
      
    return msk;
  }
  
  // Bit Slice
  //
  u32 slice(const u32 d32,  const int end_bit, const int st_bit){
    assert( (st_bit >= 0) && (st_bit <= 31) );
    assert( (end_bit >= 0) && (end_bit <= 31) );

    u32 val      = d32 & msk32bit(end_bit, st_bit);
    u32 sft_val  = val  >> st_bit;

    return sft_val;
  }

  // 32bit Sign-Extended
  //
  u32 sext32(const u32 d32, const int num_of_bits){
    assert( (num_of_bits >= 2) && (num_of_bits <= 32) );
    if( num_of_bits == 32 ){
      return(d32);
    }

    u32 sign_bit = slice(d32, (num_of_bits-1), (num_of_bits-1));

    u32 sign_ext = 0u;
    for(int i=num_of_bits; i<32; i++){
      sign_ext |= (sign_bit) << i; 
    }
    
    return( sign_ext | d32);
  }

  
// ------------------------------------------------------------
// -- RV32I Instructions
// ------------------------------------------------------------
  void op_lui(const u32 d32, bool noexec=false){    
    IMM   imm31_12 = slice(d32, 31, 12);
    REGNO rd       = slice(d32, 11,  7);

    IMM imm_sft = imm31_12 << 12;
    int imm = Int2Sign(imm_sft);
    
    std::cout << boost::format("lui x%d = %d") % rd % imm << std::endl;   
    if( noexec ) return;

    // x[rd] = sext(immediate[31:12] << 12)
    reg[rd] = imm;    

    reg.pc += 4;
  }

  void op_auipc(const u32 d32, bool noexec=false){
    IMM   imm31_12 = slice(d32, 31, 12);
    REGNO rd       = slice(d32, 11,  7);

    IMM imm = imm31_12 << 12;    
    std::cout << boost::format("auipc x%d = PC(0x%x) + 0x%x") % rd % reg.pc % imm << std::endl;
    if( noexec ) return;
    
    // [rd] = pc + sext(immediate[31:12] << 12)   
    reg[rd] = reg.pc + imm;    
    
    reg.pc+=4;
  }

  void op_jal(const u32 d32, bool noexec=false){
    IMM d20    = slice(d32, 31,31);
    IMM d10_1  = slice(d32, 30,21);
    IMM d11    = slice(d32, 20,20);
    IMM d19_12 = slice(d32, 19,12);   
    REGNO rd   = slice(d32, 11,  7);

    IMM offset =
      (d20    << 20 ) |
      (d19_12 << 12 ) |
      (d11    << 11 ) |
      (d10_1  << 1  ) ;

    std::cout << boost::format("jal x%d = PC(0x%x) + 4, PC(0x%x)+=0x%x") % rd % reg.pc % reg.pc % offset << std::endl;
    if( noexec ) return;
    
    // x[rd] = pc+4; pc += sext(offset)
    reg[rd] = reg.pc + 4 ;
    reg.pc += sext32(offset, 21);
  }

  void op_jalr(const u32 d32, bool noexec=false){
    IMM   offset = slice(d32, 31,20);
    REGNO rs1    = slice(d32, 19,15);
    REGNO rd     = slice(d32, 11,7);
    
    // t =pc+4; pc=(x[rs1]+sext(offset))&∼1; x[rd]=t
    u32 t = reg.pc + 4u;    
    std::cout << boost::format("jalr t=PC(0x%x)+4, pc=(x%d + %d) & 0xFFFFFFFE, x%d=t")  % reg.pc  % rs1 % offset % rd << std::endl;
    if( noexec ) return;
    
    reg.pc = (reg[rs1] + sext32(offset,12)) & 0xFFFFFFFE;
    reg[rd] = t;
  }

  void op_beq(const u32 d32, bool noexec=false){
    IMM   d12   = slice(d32, 31, 31); 
    IMM   d10_5 = slice(d32, 30, 25); 
    REGNO rs2   = slice(d32, 24, 20);
    REGNO rs1   = slice(d32, 19, 15);
    IMM   d4_1  = slice(d32, 11,  8);
    IMM   d11   = slice(d32,  7,  7);

    IMM offset =
      (d12   << 12) |
      (d11   << 11) |
      (d10_5 <<  5) |
      (d4_1  <<  1);
          
    std::cout << boost::format("beq if(x%d == x%d) PC(0x%x)+=%d")
                               % rs1 % rs2 % reg.pc % offset << std::endl;
    if( noexec ) return;
        
    // if (rs1 == rs2) pc += sext(offset)
    if( reg[rs1] == reg[rs2] ){
      reg.pc += sext32(offset,13);
    }else{
      reg.pc+=4;    
    }
  }

  void op_bne(const u32 d32, bool noexec=false){
    IMM   d12   = slice(d32, 31, 31); 
    IMM   d10_5 = slice(d32, 30, 25); 
    REGNO rs2   = slice(d32, 24, 20);
    REGNO rs1   = slice(d32, 19, 15);
    IMM   d4_1  = slice(d32, 11,  8);
    IMM   d11   = slice(d32,  7,  7);

    IMM offset =
      (d12   << 12) |
      (d11   << 11) |
      (d10_5 <<  5) |
      (d4_1  <<  1);          
    
    std::cout << boost::format("bne if( x%d != x%d ) pc(0x%x) += %d")
              % rs1 % rs2 % reg.pc % offset << std::endl;
    if( noexec ) return;

    // if (rs1 != rs2) pc += sext(offset)
    if( reg[rs1] != reg[rs2] ){
      reg.pc += sext32(offset,13);
    }else{
      reg.pc += 4;    
    }
  }


  void op_blt(const u32 d32, bool noexec=false){
    std::cout << "blt" << std::endl;
    if( noexec ) return;
    
    IMM   d12   = slice(d32, 31, 31); 
    IMM   d10_5 = slice(d32, 30, 25); 
    REGNO rs2   = slice(d32, 24, 20);
    REGNO rs1   = slice(d32, 19, 15);
    IMM   d4_1  = slice(d32, 11,  8);
    IMM   d11   = slice(d32,  7,  7);

    IMM offset =
      (d12   << 12) |
      (d11   << 11) |
      (d10_5 <<  5) |
      (d4_1  <<  1);          
    
    // if (rs1 <s rs2) pc += sext(offset)
    if( Int2Sign(reg[rs1]) < Int2Sign(reg[rs2]) ){
      reg.pc += sext32(offset,12);
    }else{
      reg.pc+=4;    
    }
  }
  
  void op_bge(const u32 d32, bool noexec=false){   
    IMM   d12   = slice(d32, 31, 31); 
    IMM   d10_5 = slice(d32, 30, 25); 
    REGNO rs2   = slice(d32, 24, 20);
    REGNO rs1   = slice(d32, 19, 15);
    IMM   d4_1  = slice(d32, 11,  8);
    IMM   d11   = slice(d32,  7,  7);

    IMM offset =
      (d12   << 12) |
      (d11   << 11) |
      (d10_5 <<  5) |
      (d4_1  <<  1);          
    
    // if (rs1 >=s rs2) pc += sext(offset)
    std::cout << boost::format("bge  if(x%d  >=s x%d) PC(0x%x) + sext(%d)")
                               % rs1 % rs2 % reg.pc % offset << std::endl;
    if( noexec ) return;

    if( Int2Sign(reg[rs1]) >= Int2Sign(reg[rs2]) ){
      reg.pc += sext32(offset,13);
    }else{
      reg.pc+=4;    
    }
  }
  
  void op_bltu(const u32 d32, bool noexec=false){
    IMM   d12   = slice(d32, 31, 31); 
    IMM   d10_5 = slice(d32, 30, 25); 
    REGNO rs2   = slice(d32, 24, 20);
    REGNO rs1   = slice(d32, 19, 15);
    IMM   d4_1  = slice(d32, 11,  8);
    IMM   d11   = slice(d32,  7,  7);

    IMM offset =
      (d12   << 12) |
      (d11   << 11) |
      (d10_5 <<  5) |
      (d4_1  <<  1);          
    
    std::cout << boost::format("bltu if(x%d <u x%d) PC(0x%x) += sext(%d)")
              % rs1 % rs2 % reg.pc % offset << std::endl;
    if( noexec ) return;
    
    // if (rs1 <u rs2) pc += sext(offset)
    if( reg[rs1] < reg[rs2] ){
      reg.pc += sext32(offset,12);
    }else{
      reg.pc+=4;    
    }
  }
  
  void op_bgeu(const u32 d32, bool noexec=false){
    std::cout << "bgeu" << std::endl;
    if( noexec ) return;
    
    IMM   d12   = slice(d32, 31, 31); 
    IMM   d10_5 = slice(d32, 30, 25); 
    REGNO rs2   = slice(d32, 24, 20);
    REGNO rs1   = slice(d32, 19, 15);
    IMM   d4_1  = slice(d32, 11,  8);
    IMM   d11   = slice(d32,  7,  7);

    IMM offset =
      (d12   << 12) |
      (d11   << 11) |
      (d10_5 <<  5) |
      (d4_1  <<  1);          
    
    // if (rs1 >=u rs2) pc += sext(offset)
    if( reg[rs1] >= reg[rs2] ){
      reg.pc += sext32(offset,13);
    }else{
      reg.pc+=4;    
    }
  }
  
  void op_lb(const u32 d32, bool noexec=false){
    IMM   offset = slice(d32, 31, 20);
    REGNO rs1    = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);


    ADDR addr = (reg[rs1] + sext32(offset, 12));
    u32 mem_data32  = MEM.at(addr >> 2);
    u32 byte_offset = slice(addr,1,0); // Addr LSB 2bit  
    u32 d8 = (byte_offset==0 )  ? slice(mem_data32,  7,  0) :
              (byte_offset==1 )  ? slice(mem_data32, 15,  8) :
              (byte_offset==2 )  ? slice(mem_data32, 23,  16)
            /*(byte_offset==3 )*/: slice(mem_data32, 31,  24);
    
    std::cout << boost::format("lb x%d = sext( MEM[x%d + sext(%d)][7:0]),    MEM(0x%08x)=0x08%x")
                          % rd % rs1 % offset % addr % MEM.at(addr>>2) << std::endl;
    if( noexec ) return;
    

    // x[rd] = sext(M[x[rs1] + sext(offset)][7:0])
    reg[rd] = sext32(d8,8);
    
    reg.pc+=4;
  }

  void op_lh(const u32 d32, bool noexec=false){
    IMM   offset = slice(d32, 31, 20);
    REGNO rs1    = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);

    ADDR addr = (reg[rs1] + sext32(offset, 12));  

    // 32bit中の2byteを選択
    u32 mem_data32  = MEM.at(addr >> 2);
    u32 byte_offset = slice(addr,1,0);  // Addr LSB 2bit  
    u32 d16         = (byte_offset==0 )  ? slice(mem_data32, 15,  0) :
                      (byte_offset==2 )  ? slice(mem_data32, 31, 16)
                                         : 0u;

    std::cout << boost::format("lh x%d = sext( MEM[x%d + sext(%d)][15:0]),    MEM(0x%08x)=0x%08x")
                          % rd % rs1 % offset % addr % MEM.at(addr>>2) << std::endl;
    if( noexec ) return;
    
    // x[rd] = sext(M[x[rs1] + sext(offset)][15:0])
                     
    // Register Write
    reg[rd] = sext32(d16,16);
    
    reg.pc+=4;
  }

  void op_lw(const u32 d32, bool noexec=false){
    IMM   offset = slice(d32, 31, 20);
    REGNO rs1    = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);

    std::cout << boost::format("lw  x%d = sext( MEM[x%d + sext(%d)])")
                               % rd % rs1 % offset << std::endl;
    if( noexec ) return;
    
    // x[rd] = sext(M[x[rs1] + sext(offset)][31:0])
    reg[rd] = slice( MEM.at((reg[rs1] + sext32(offset,12)) >> 2), 
                      31,0);
    
    reg.pc+=4;
  }

  void op_lbu(const u32 d32, bool noexec=false){
    IMM   offset = slice(d32, 31, 20);
    REGNO rs1    = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);

    ADDR addr = (reg[rs1] + sext32(offset, 12));
    u32 mem_data32  = MEM.at(addr >> 2);
    u32 byte_offset = slice(addr,1,0); // Addr LSB 2bit  
    u32 d8 = (byte_offset==0 )  ? slice(mem_data32,  7,  0) :
             (byte_offset==1 )  ? slice(mem_data32, 15,  8) :
             (byte_offset==2 )  ? slice(mem_data32, 23,  16)
           /*(byte_offset==3 )*/: slice(mem_data32, 31,  24);

    std::cout << boost::format("lb x%d = sext( MEM[x%d + sext(%d)][7:0]),    MEM(0x%08x)=0x08%x")
                          % rd % rs1 % offset % addr % MEM.at(addr>>2) << std::endl;
    if( noexec ) return;
    
    // x[rd] = M[x[rs1] + sext(offset)][15:0]
    reg[rd] = d8 & 0xFF;
    
    reg.pc+=4;
  }

  void op_lhu(const u32 d32, bool noexec=false){
    IMM   offset = slice(d32, 31, 20);
    REGNO rs1    = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);

    ADDR addr = (reg[rs1] + sext32(offset, 12));  

    // 32bit中の2byteを選択
    u32 mem_data32  = MEM.at(addr >> 2);
    u32 byte_offset = slice(addr,1,0);  // Addr LSB 2bit  
    u32 d16         = (byte_offset==0 )  ? slice(mem_data32, 15,  0) :
                      (byte_offset==2 )  ? slice(mem_data32, 31, 16)
                                         : 0u;

    std::cout << boost::format("lh x%d = sext( MEM[x%d + sext(%d)][15:0]),    MEM(0x%08x)=0x%08x")
                          % rd % rs1 % offset % addr % MEM.at(addr>>2) << std::endl;
    if( noexec ) return;
    
    // x[rd] = M[x[rs1] + sext(offset)][15:0]
    reg[rd] = d16 & 0xFFFF;
    
    reg.pc+=4;
  }

  void op_sb(const u32 d32, bool noexec=false){    
    IMM   d11_5 = slice(d32,31,25);
    REGNO rs2   = slice(d32,24,20);
    REGNO rs1   = slice(d32,19,15);
    IMM   d4_0  = slice(d32,11, 7);

    IMM offset = (d11_5 << 5) |
                 d4_0 ;    

    ADDR addr        = reg[rs1] + sext32(offset,12);    
    ADDR byte_offset = slice(addr,1,0);  // Addr LSB 2bit  
    u32  mem_data32  = MEM.at(addr >> 2);

    std::cout << boost::format("sb MEM[x%d + sext(%d)] = x%d[7:0],    MEM(0x%08x)=0x%08x") 
                         % rs1 % offset % rs2 % addr % mem_data32 << std::endl;      
    if( noexec ){ return; }

    //M[x[rs1] + sext(offset)] = x[rs2][7:0]
    switch( byte_offset ){
    case 0 : MEM[addr >> 2] = (mem_data32 & 0xFFFFFF00) |  slice(reg[rs2], 7, 0)       ; break;
    case 1 : MEM[addr >> 2] = (mem_data32 & 0xFFFF00FF) | (slice(reg[rs2], 7, 0) << 8) ; break;
    case 2 : MEM[addr >> 2] = (mem_data32 & 0xFF00FFFF) | (slice(reg[rs2], 7, 0) << 16); break;
    case 3 : MEM[addr >> 2] = (mem_data32 & 0x00FFFFFF) | (slice(reg[rs2], 7, 0) << 24); break;
    } 

    reg.pc+=4;
  }

  void op_sh(const u32 d32, bool noexec=false){
    IMM   d11_5 = slice(d32,31,25);
    REGNO rs2   = slice(d32,24,20);
    REGNO rs1   = slice(d32,19,15);
    IMM   d4_0  = slice(d32,11, 7);

    IMM offset = (d11_5 << 5) |
                 d4_0 ;
    
    ADDR addr        = reg[rs1] + sext32(offset,12);    
    ADDR byte_offset = slice(addr,1,0);  // Addr LSB 2bit  
    u32  mem_data32  = MEM.at(addr >> 2);

    std::cout << boost::format("sh MEM[x%d + sext(%d)] = x%d[15:0],    MEM(0x%08x)=0x%08x") 
                         % rs1 % offset % rs2 % addr % mem_data32 << std::endl;
    if( noexec ) return;

    //M[x[rs1] + sext(offset)] = x[rs2][15:0]
    switch( byte_offset ){
    case 0 : MEM[addr >> 2] = (mem_data32 & 0xFFFF0000) |  slice(reg[rs2], 15, 0)       ; break;
    case 2 : MEM[addr >> 2] = (mem_data32 & 0x0000FFFF) | (slice(reg[rs2], 15, 0) << 16); break;
    } 

    reg.pc+=4;
  }

  void op_sw(const u32 d32, bool noexec=false){
    IMM d11_5=slice(d32,31,25);
    REGNO rs2=slice(d32,24,20);
    REGNO rs1=slice(d32,19,15);
    IMM d4_0 =slice(d32,11, 7);

    IMM offset = (d11_5 << 5) |
                 d4_0 ;
    
    ADDR addr = reg[rs1] + sext32(offset,12);

    std::cout << boost::format("sw  mem[x%d + %d] = x%d,   addr=%x")
                              % rs1 % Int2Sign(offset) % rs2  % addr  << std::endl;
    if( noexec ) return;    

    //M[x[rs1] + sext(offset)] = x[rs2][31:0]
    MEM[addr >> 2] = slice(reg[rs2],31,0); 
    
    reg.pc+=4;
  }

  void op_addi(const u32 d32, bool noexec=false){    
    IMM   imm11_0 = slice(d32,31,20);
    REGNO rs1     = slice(d32,19,15);
    REGNO rd      = slice(d32,11, 7);
    std::cout << boost::format("addi  x%d = x%d + %d") % rd % rs1 % sext32(imm11_0,12) << std::endl;
    if( noexec ) return;

    //x[rd] = x[rs1] + sext(immediate)
    reg[rd] = reg[rs1] + sext32(imm11_0,12);
    
    reg.pc+=4;
  }

  void op_slti( u32 d32, bool noexec=false){
    std::cout << "slti" << std::endl;
    if( noexec ) return;
    
    IMM imm11_0=slice(d32,31,20);
    REGNO rs1=slice(d32,19,15);
    REGNO rd =slice(d32,11, 7);

    //x[rd] = x[rs1] <s sext(immediate)
    reg[rd] = (Int2Sign(reg[rs1]) < Int2Sign(sext32(imm11_0,12))) ? 1 : 0;
    
    reg.pc+=4;
  }

  void op_sltiu(const u32 d32, bool noexec=false){
    std::cout << "sltiu" << std::endl;
    if( noexec ) return;
    
    IMM   imm11_0= slice(d32,31,20);
    REGNO rs1    = slice(d32,19,15);
    REGNO rd     = slice(d32,11, 7);

    //x[rd] = x[rs1] <s sext(immediate)
    reg[rd] = (reg[rs1] < sext32(imm11_0,12) ) ? 1 : 0;
    
    reg.pc+=4;
  }

  void op_xori(const u32 d32, bool noexec=false){
    std::cout << "xori" << std::endl;
    if( noexec ) return;
    
    IMM imm11_0=slice(d32,31,20);
    REGNO rs1=slice(d32,19,15);
    REGNO rd =slice(d32,11, 7);

    //x[rd] = x[rs1] ^ sext(immediate)
    reg[rd] = reg[rs1] ^  sext32(imm11_0,12);
    
    reg.pc+=4;
  }

  void op_ori(const u32 d32, bool noexec=false){
    std::cout << "ori" << std::endl;
    if( noexec ) return;
    
    IMM imm11_0=slice(d32,31,20);
    REGNO rs1=slice(d32,19,15);
    REGNO rd =slice(d32,11, 7);

    //x[rd] = x[rs1] | sext(immediate)
    reg[rd] = reg[rs1] | sext32(imm11_0, 12);  
    
    reg.pc+=4;
  }

  void op_andi(const u32 d32, bool noexec=false){
    std::cout << "andi" << std::endl;
    if( noexec ) return;
    
    IMM imm11_0=slice(d32,31,20);
    REGNO rs1=slice(d32,19,15);
    REGNO rd =slice(d32,11, 7);

    //x[rd] = x[rs1] & sext(immediate)
    reg[rd] = reg[rs1] & sext32(imm11_0, 12);
    
    reg.pc+=4;
  }

  void op_slli(const u32 d32, bool noexec=false){
    IMM shamt=slice(d32,24,20);
    REGNO rs1=slice(d32,19,15);
    REGNO rd =slice(d32,11, 7);

    
    std::cout << boost::format("slli x%d = x%d << %d") % rd % rs1 % shamt << std::endl;
    if( noexec ) return;    

    //x[rd] = x[rs1] << shamt
    reg[rd] = reg[rs1] << shamt;
    
    reg.pc+=4;
  }

  void op_srli(const u32 d32, bool noexec=false){
    std::cout << "srli" << std::endl;
    if( noexec ) return;
    
    IMM shamt=slice(d32,24,20);
    REGNO rs1=slice(d32,19,15);
    REGNO rd =slice(d32,11, 7);

    //x[rd] = x[rs1] >> shamt
    reg[rd] = reg[rs1] >> shamt;
    
    reg.pc+=4;
  }

  void op_srai(const u32 d32, bool noexec=false){
    std::cout << "srai" << std::endl;
    if( noexec ) return;
    
    IMM   shamt=slice(d32,24,20);
    REGNO rs1=slice(d32,19,15);
    REGNO rd =slice(d32,11, 7);

    //x[rd] = x[rs1] >>s shamt
    reg[rd] = Int2Sign(reg[rs1]) >> shamt; // arithmetic right shift o
    
    reg.pc+=4;
  }

  void op_add(const u32 d32, bool noexec=false){
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    std::cout << boost::format("add x%d = x%d + x%d")
                     % rd % rs1 % rs2 << std::endl;
    if( noexec ) return;
    
    // x[rd] = x[rs1] + x[rs2]
    reg[rd] = reg[rs1] + reg[rs2]; 
    
    reg.pc+=4;
  }

  void op_mul(const u32 d32, bool noexec=false){
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    std::cout << boost::format("mul x%d = x%d + x%d")
                     % rd % rs1 % rs2 << std::endl;
    if( noexec ) return;
    
    // x[rd] = x[rs1] * x[rs2]
    reg[rd] = reg[rs1] * reg[rs2]; 
    
    reg.pc+=4;
  }

  void op_sub(const u32 d32, bool noexec=false){
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    std::cout << boost::format("sub  x%d = x%d - x%d") % rd % rs1 % rs2 << std::endl;
    if( noexec ) return;
    
    // x[rd] = x[rs1] - x[rs2]
    reg[rd] = reg[rs1] - reg[rs2]; 
    
    reg.pc+=4;
  }

  void op_sll(const u32 d32, bool noexec=false){
    std::cout << "sll" << std::endl;
    if( noexec ) return;
    
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    // x[rd] = x[rs1] << x[rs2]
    reg[rd] = reg[rs1] << reg[rs2]; 
    
    reg.pc+=4;
  }

  void op_slt(const u32 d32, bool noexec=false){
    std::cout << "slt" << std::endl;
    if( noexec ) return;
    
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    // x[rd] = x[rs1] <s x[rs2]
    reg[rd] = (Int2Sign(reg[rs1]) < Int2Sign(reg[rs2])) ? 1 : 0; 
    
    reg.pc+=4;
  }

  void op_sltu(const u32 d32, bool noexec=false){
    std::cout << "sltu" << std::endl;
    if( noexec ) return;
    
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    // x[rd] = x[rs1] <u x[rs2]
    reg[rd] = (reg[rs1] < reg[rs2]) ? 1 : 0; 
    
    reg.pc+=4;
  }

  void op_xor(const u32 d32, bool noexec=false){
    std::cout << "xor" << std::endl;
    if( noexec ) return;
    
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    // x[rd] = x[rs1] ^ x[rs2]
    reg[rd] = reg[rs1] ^ reg[rs2]; 
    
    reg.pc+=4;
  }

  void op_div(const u32 d32, bool noexec=false){
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    std::cout << boost::format("div x%d = x%d / x%d") % rd % rs1 % rs2 << std::endl;
    if( noexec ) return;
    
    // x[rd] = x[rs1] / x[rs2]
    reg[rd] = reg[rs1] / reg[rs2]; 
    
    reg.pc+=4;
  }

  void op_srl(const u32 d32, bool noexec=false){
    std::cout << "srl" << std::endl;
    if( noexec ) return;
    
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    //  x[rs1] >>u x[rs2]
    reg[rd] = reg[rs1] >> reg[rs2]; 
    
    reg.pc+=4;
  }

  void op_sra(const u32 d32, bool noexec=false){
    std::cout << "sra" << std::endl;
    if( noexec ) return;
    
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    //  x[rd] = x[rs1] >>s x[rs2]
    reg[rd] = Int2Sign(reg[rs1]) >> reg[rs2]; // arithmetic right shift 
    
    reg.pc+=4;
  }

  void op_divu(const u32 d32, bool noexec=false){
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    std::cout << boost::format("divu  x%d = x%d / x%d")
                               % rd % rs1 % rs2 << std::endl;
    if( noexec ) return;    

    // x[rd] = x[rs1] /u x[rs2]
    reg[rd] = reg[rs1] / reg[rs2];
    
    reg.pc+=4;
  }

  void op_or(const u32 d32, bool noexec=false){
    std::cout << "or" << std::endl;
    if( noexec ) return;
    
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    //  x[rd] = x[rs1] | x[rs2]
    reg[rd] = reg[rs1] | reg[rs2]; 
    
    reg.pc+=4;
  }

  void op_and(const u32 d32, bool noexec=false){
    std::cout << "and" << std::endl;
    if( noexec ) return;
    
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    //  x[rd] = x[rs1] | x[rs2]
    reg[rd] = reg[rs1] & reg[rs2]; 
    
    reg.pc+=4;
  }

  void op_remu(const u32 d32, bool noexec){ // RV32M
    REGNO rs2 = slice(d32, 24, 20);
    REGNO rs1 = slice(d32, 19, 15);
    REGNO rd  = slice(d32, 11,  7);

    std::cout << boost::format("remu  x%d = x%d %% x%d")
                               % rd % rs1 % rs2 << std::endl;
    if( noexec ) return;    

    // x[rd] = x[rs1] %u x[rs2]    
    reg[rd] = reg[rs1] % reg[rs2]; 

                               
    reg.pc+=4;        
  }

  void op_fence(const u32 d32, bool noexec=false){
    std::cout << "fence" << std::endl;
    if( noexec ) return;
    
    reg.pc+=4;    
  }

  void op_fence_I(const u32 d32, bool noexec=false){
    std::cout << "fence.I" << std::endl;
    if( noexec ) return;
    
    reg.pc+=4;    
  }

  void op_ecall(const u32 d32, bool noexec=false){
    std::cout << "ECALL" << std::endl;
    if( noexec ) return;
    
    reg.report();
    exit(0);
    ////reg.pc+=4;    
  }

  void op_ebreak(const u32 d32, bool noexec=false){
    std::cout << "EBREAK" << std::endl;
    if( noexec ) return;

    std::cout << "...FIN" << std::endl;
    reg.report();
    exit(0);
    ////reg.pc+=4;    
  }

  void op_mret(const u32 d32, bool noexec=false){
    std::cout << "mret" << std::endl;
    if( noexec ) return;

    reg.pc+=4;    
  }
  
  void op_csrrw(const u32 d32, bool noexec=false){
    IMM   offset = slice(d32, 31, 20);
    REGNO rs1    = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);
    
    std::cout << boost::format("csrrw t = CSRs[0x%x]; CSRs[0x%x] = x[x%d]; x[x%d] = t")
              % offset % offset % rs1 % rd << std::endl;
    if( noexec ) return;

    // t = CSRs[csr]; CSRs[csr] = x[rs1]; x[rd] = t
    IMM t = CSRs[offset];
    CSRs[offset] = reg[rs1];
    reg[rd] = t;

    reg.pc+=4;
  }

  void op_csrrs(const u32 d32, bool noexec=false){
    REGNO offset = slice(d32, 31, 20);
    REGNO rs1    = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);

    std::cout << boost::format("csrrs t = CSRs[0x%x]; CSRs[0x%x] = t | x[x%d]; x[x%d] = t")
              % offset % offset % rs1 % rd << std::endl;
    if( noexec ) return;
    
    //t = CSRs[csr]; CSRs[csr] = t | x[rs1]; x[rd] = t
    IMM t = CSRs[offset];
    CSRs[offset] = t | reg[rs1];
    reg[rd] = t;
    
    reg.pc+=4;
  }
  
  void op_csrrc(const u32 d32, bool noexec=false){
    REGNO offset = slice(d32, 31, 20);
    REGNO rs1    = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);

    std::cout << boost::format("csrrc t = CSRs[0x%x]; CSRs[0x%x] = t | x[x%d]; x[x%d] = t")
              % offset % offset % rs1 % rd << std::endl;
    if( noexec ) return;
    
    //t = CSRs[csr]; CSRs[csr] = t &∼x[rs1]; x[rd] = t
    IMM t = CSRs[offset];
    CSRs[offset] = t & (~reg[rs1]);
    reg[rd] = t;
    
    reg.pc+=4;
  }

  void op_csrrwi(const u32 d32, bool noexec=false){
    REGNO offset = slice(d32, 31, 20);
    REGNO zimm   = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);

    std::cout << boost::format("csrrwi  x[x%d] = CSRs[0x%x]; CSRs[0x%x] = 0x%x")
              % rd  % offset % offset % zimm << std::endl;
    if( noexec ) return;
    
    // x[rd] = CSRs[csr]; CSRs[csr] = zimm 
    reg[rd] = CSRs[offset];
    CSRs[offset] = zimm;
    
    reg.pc+=4;
  }

  void op_csrrsi(const u32 d32, bool noexec=false){
    REGNO offset = slice(d32, 31, 20);
    REGNO zimm   = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);

    std::cout << boost::format("csrrsi  t = CSRs[0x%x]; CSRs[0x%x] = t | 0x%x; x%d = t")
              % offset % offset % zimm % rd << std::endl;
    if( noexec ) return;
    
    // t = CSRs[csr]; CSRs[csr] = t | zimm; x[rd] = t
    IMM t = CSRs[offset];
    CSRs[offset]= t | zimm; 
    reg[rd] = t;
    
    reg.pc+=4;
  }
  
  void op_csrrci(const u32 d32, bool noexec=false){
    REGNO offset = slice(d32, 31, 20);
    REGNO zimm   = slice(d32, 19, 15);
    REGNO rd     = slice(d32, 11,  7);

    std::cout << boost::format("csrrci  t = CSRs[0x%x]; CSRs[0x%x] = t & ~(0x%x); x[x%d] = t")
              % offset % offset % zimm % rd << std::endl;
    if( noexec ) return;
    
    // t = CSRs[csr]; CSRs[csr] = t &∼zimm; x[rd] = t
    IMM t = CSRs[offset];
    CSRs[offset]= t & ~zimm; 
    reg[rd] = t;
    
    reg.pc+=4;
  }


// ------------------------------------------------------------
// -- Decoder
// ------------------------------------------------------------

  //
  // CFigura 2.3: El mapa de opcodes de RV32I tiene la estructura de la inst
  //
  // funct7  fuct3   opecode 
  // ------- ---     ------- - --------- 
  //                 0110111 U lui
  //                 0010111 U auipc
  //                 1101111 J jal
  //         000     1100111 I jalr
  //
  //         000     1100011 B beq
  //         001     1100011 B bne
  //         100     1100011 B blt
  //         101     1100011 B bge
  //         110     1100011 B bltu
  //         111     1100011 B bgeu
  //
  //         000     0000011 I lb
  //         001     0000011 I lh
  //         010     0000011 I lw
  //         100     0000011 I lbu
  //         101     0000011 I lhu
  //
  //         000     0100011 S sb
  //         001     0100011 S sh
  //         010     0100011 S sw
  //
  //         000     0010011 I addi
  //         010     0010011 I slti
  //         011     0010011 I sltiu
  //         100     0010011 I xori
  //         110     0010011 I ori
  //         111     0010011 I andi
  // 0000000 001     0010011 I slli
  // 0000000 101     0010011 I srli
  // 0100000 101     0010011 I srai
  //
  // 0000000 000     0110011 R add
  // 0000001 000     0110011 R mul (*)
  // 0100000 000     0110011 R sub
  // 0000000 001     0110011 R sll
  // 0000000 010     0110011 R slt
  // 0000000 011     0110011 R sltu
  // 0000000 100     0110011 R xor
  // 0000000 101     0110011 R srl
  // 0100000 101     0110011 R sra
  // 0000000 110     0110011 R or
  // 0000000 111     0110011 R and
  //
  // 0000001 111     0110011 remu
  //
  // 0000 pred succ 00000 000 00000 0001111 I fence
  // 0000 0000 0000 00000 001 00000 0001111 I fence.i
  // 000000000000 00000 000 00000 1110011 I ecall
  // 000000000001 00000 000 00000 1110011 I ebreak
  // csr     001     1110011 I csrrw
  // csr     010     1110011 I csrrs
  // csr     011     1110011 I csrrc
  // csr     101     1110011 I csrrwi
  // csr     110     1110011 I csrrsi
  // csr     111     1110011 I csrrci
 
  void __decode_err( IMM opecode, IMM funct3, IMM funct7){
     std::cout << boost::format("[ERR] NOT SUPPORT OPECODE ope(0x%02X) funct3(0x%X)  funct7(0x%X)") % opecode % funct3 % funct7  << std::endl;
     exit(0);
  }
  
  void decode(u32 d32, bool noexec=false){
    IMM funct7   = slice(d32,31,25);
    IMM funct3   = slice(d32,14,12);
    IMM opecode  = slice(d32, 6, 0);
    IMM imm31_20 = slice(d32,31,20);

    // 2進数表記は，c++14以降
    switch(opecode){
    case 0b0110111 : //------------------------------
      op_lui(d32, noexec); break;
    case 0b0010111 : //------------------------------
      op_auipc(d32, noexec); break;
    case 0b1101111 : //------------------------------
      op_jal(d32, noexec);    break;
    case 0b1100111 : //------------------------------
      if( funct3 == 0b000 ){ op_jalr(d32, noexec); break; }
      else{        __decode_err(opecode, funct3, funct7); break;}
      break;
    case 0b1100011 : //------------------------------
      switch ( funct3 ){
      case 0b000 : op_beq(d32, noexec); break;
      case 0b001 : op_bne(d32, noexec); break;
      case 0b100 : op_blt(d32, noexec); break;
      case 0b101 : op_bge(d32, noexec); break;
      case 0b110 : op_bltu(d32, noexec); break; 
      case 0b111 : op_bgeu(d32, noexec); break;
      default    : __decode_err(opecode, funct3, funct7); break;
      }
      break;
    case 0b0000011 : //------------------------------
      switch( funct3 ){
      case 0b000: op_lb(d32, noexec); break;
      case 0b001: op_lh(d32, noexec); break;
      case 0b010: op_lw(d32, noexec); break;
      case 0b100: op_lbu(d32, noexec); break;
      case 0b101: op_lhu(d32, noexec); break;
      default: __decode_err(opecode, funct3, funct7); break;
      }
      break;
    case 0b0100011: //------------------------------
      switch( funct3 ){
      case 0b000: op_sb(d32, noexec); break;
      case 0b001: op_sh(d32, noexec); break;
      case 0b010: op_sw(d32, noexec); break;
      default: __decode_err(opecode, funct3, funct7); break;
      }
      break;
    case 0b0010011 : //------------------------------
      switch( funct3 ){      
      case 0b000: op_addi(d32, noexec); break;
      case 0b010: op_slti(d32, noexec); break;
      case 0b011: op_sltiu(d32, noexec); break;
      case 0b100: op_xori(d32, noexec); break;
      case 0b110: op_ori(d32, noexec); break;
      case 0b111: op_andi(d32, noexec); break;
      case 0b001:
        if( funct7 == 0b0000000 ) op_slli(d32, noexec); 
        else { __decode_err(opecode, funct3, funct7); break; }
        break;
      case 0b101:
        switch( funct7 ){
        case 0b0000000 : 
          if( funct7 == 0b0000000 ){ op_srli(d32, noexec);  break;}
          else { __decode_err(opecode, funct3, funct7); break;}
          break;
        case 0b0100000 : 
          if( funct7 == 0b0100000 ) op_srai(d32, noexec);
          else { __decode_err(opecode, funct3, funct7); break;}
          break;
        default: __decode_err(opecode, funct3, funct7); break;
        }
        break;
      default: __decode_err(opecode, funct3, funct7); break;
      }
      break;
    case 0b0110011 : //------------------------------
      switch( funct3 ){
      case 0b000: 
        switch( funct7 ){
        case 0b0000000:{ op_add(d32, noexec); break; }
        case 0b0100000:{ op_sub(d32, noexec); break; }
        case 0b0000001:{ op_mul(d32, noexec); break; }
        default       :{ __decode_err(opecode, funct3, funct7); break; }
        }
        break;
      case 0b001:
        if( funct7 == 0b0000000 ) op_sll(d32, noexec);
        else { __decode_err(opecode, funct3, funct7); break; }
        break;        
      case 0b010: 
        if( funct7 == 0b0000000 ) op_slt(d32, noexec);
        else { __decode_err(opecode, funct3, funct7); break; }
        break;
      case 0b011:
        if( funct7 == 0b0000000 ) op_sltu(d32, noexec);
        else { __decode_err(opecode, funct3, funct7); break; }
        break;
      case 0b100: 
        switch(funct7){
        case 0b0000000: { op_xor(d32, noexec); break; }
        case 0b0000001: { op_div(d32, noexec); break; } 
        default: { __decode_err(opecode, funct3, funct7); break; }
        }
        break;
      case 0b101:
        switch( funct7 ){
        case 0b0000000: op_srl(d32, noexec); break;
        case 0b0100000: op_sra(d32, noexec); break;
        case 0b0000001: op_divu(d32, noexec); break;
        default: __decode_err(opecode, funct3, funct7); break;
        }
        break;
      case 0b110: 
        if( funct7 == 0b0000000 ) op_or(d32, noexec);
        else { __decode_err(opecode, funct3, funct7); break; }
        break;
      case 0b111: 
        switch( funct7 ){
        case 0b0000000: op_and(d32, noexec); break;
        case 0b0000001: op_remu(d32, noexec); break;
        default       : __decode_err(opecode, funct3, funct7); break;
        }
      }
      break;
    case 0b0001111: //------------------------------
      switch( funct3 ){
      case 0b000: op_fence(d32, noexec); break;
      case 0b001: op_fence_I(d32, noexec); break;
      default: __decode_err(opecode, funct3, funct7); break;
      }
      break;

    case 0b1110011: //------------------------------

      switch( funct3 ){
      case 0b000:
        switch(imm31_20){ 
        case 0b000000000000: op_ecall(d32, noexec); break;
        case 0b000000000001: op_ebreak(d32, noexec); break;
        case 0b001100000010: op_mret(d32, noexec); break;
        default: __decode_err(opecode, funct3, funct7); break;
        }
        break;      
      case 0b001: op_csrrw(d32, noexec); break; 
      case 0b010: op_csrrs(d32, noexec); break; 
      case 0b011: op_csrrc(d32, noexec); break; 
      case 0b101: op_csrrwi(d32, noexec); break; 
      case 0b110: op_csrrsi(d32, noexec); break; 
      case 0b111: op_csrrci(d32, noexec); break;
      default: __decode_err(opecode, funct3, funct7); break;
      }
      break;

    default:  //-----------------------------------
      __decode_err(opecode, funct3, funct7); break;    
    }

    /// 強制的に０設定
    reg[0] = 0;
    //printf("STOP 0x00026364 0x%x\n", MEM.at(0x00026364 >> 2));
  }


// ------------------------------------------------------------
// -- Read Memory Image
// ------------------------------------------------------------
  void read_binimgtxt(char *binary_image_txt){
    //std::cout << "======================================================" << std::endl;
    //std::cout << "== " << binary_image_txt << std::endl;
    //std::cout << "======================================================" << std::endl;
    
    std::ifstream imgfile;
    std::string   buf;
    
    int addr = 0;
    // int addr_max =0;
    imgfile.open(binary_image_txt, std::ios::in);
    while( getline(imgfile,buf) ){
      int b0,b1,b2,b3;

      // Endian ...
      sscanf(buf.substr(0,2).c_str(), "%02x", &b0);
      sscanf(buf.substr(2,2).c_str(), "%02x", &b1);
      sscanf(buf.substr(4,2).c_str(), "%02x", &b2);
      sscanf(buf.substr(6,2).c_str(), "%02x", &b3);
      
      rvemu::uint d32 = (b3 << 24) |
                        (b2 << 16) |
                        (b1 <<  8) |
                         b0 ;
      rvemu::MEM[addr++] = d32;
    }
    //addr_max = addr;

  }

  
// ------------------------------------------------------------
// -- Execute
// ------------------------------------------------------------
  void exec(bool interactive_mode=false, ADDR break_pt_addr=0){
    reg.pc = 0; 
    int ct =0;

    bool flg_interactive = false;

    if( interactive_mode ){
      if( break_pt_addr == 0 ){
        flg_interactive = true;
      }else{
        flg_interactive = false;
      }
    }

    u32 d32 = MEM[reg.pc >> 2];
    while(1){
      //
      if( interactive_mode & (reg.pc == break_pt_addr) ){
        flg_interactive = true;
      }            

      if( flg_interactive ){
        std::string cmd;

        reg.report();
        CSRs.report();

        while(1){          
          std::cout << " r / s / m<0x***> ? >";
          std::cin >> cmd;
          if       ( cmd[0] == 'r' ){ // Continue..
            flg_interactive = false;
            break;
          }else if ( cmd[0] == 'm' ){ // MEMORY Image Report
            std::cin >> cmd;
            // Report Memory 
            ADDR mem_addr = strtoll(cmd.c_str(),NULL,16);
            std::cout << boost::format("%08X 0x%08x") % mem_addr % MEM.at(mem_addr>>2) << std::endl;
            continue;
          }

          break;
        }
      }

      ///
      std::cout << boost::format("#%d [%08x] %08x  ") % ++ct  % reg.pc % d32;
      decode(d32);
      d32 = MEM[reg.pc >> 2];

    } // while(1){
    
  }
  

}; // namespace rvemu{


//----------------------------------------------------------------------
// Main
//----------------------------------------------------------------------
#include<string>

int main(int argc, char *argv[]){
  std::ifstream ifs;
  bool interactive_mode=false;

  rvemu::ADDR break_pt_addr=0;
  
  if( argc < 2 ){
    std::cout << "> rv32uiemu <BinaryImage text>" << std::endl;
    std::cout << "> rv32uiemu <BinaryImage text> i            ... Single Step" << std::endl;
    std::cout << "> rv32uiemu <BinaryImage text> i <Address>  ... Run untill the Address + , Single Step.." << std::endl;
    return 0;
  }

  if( argc >= 3 ){
    std::string param = argv[2];
    if( argc >= 4 ) {
      break_pt_addr = strtoll(argv[3],NULL,16);  // String --> hex 
    }

    if( param == "i" ){
      std::cout << "<< INTERACTIVE >>" << std::endl;
      interactive_mode=true;
    }
  }


  //
  rvemu::read_binimgtxt(argv[1]);
  rvemu::exec(interactive_mode, break_pt_addr);  
}

