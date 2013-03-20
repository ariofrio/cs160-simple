#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>
#define forall(iterator,listptr) \
	for(iterator = listptr->begin(); iterator != listptr->end(); iterator++) \

#define print(...) fprintf(m_outputfile, __VA_ARGS__);
#define echo(...) print(__VA_ARGS__); print("\n");

class Codegen : public Visitor
{
	private:

	FILE * m_outputfile;
	SymTab *m_st;

	// basic size of a word (integers and booleans) in bytes
	static const int wordsize = 4;

	int label_count; //access with new_label

	// ********** Helper functions ********************************

	// this is used to get new unique labels (cleverly named label1, label2, ...)
	int new_label() { return label_count++; }

	// this mode is used for the code
	void set_text_mode() { fprintf( m_outputfile, ".text\n\n"); }
	
	// PART 1:
	// 1) get arithmetic expressions on integers working:
	//	  you wont really be able to run your code,
	//	  but you can visually inspect it to see that the correct
	//    chains of opcodes are being generated.
	// 2) get function calls working:
	//    if you want to see at least a very simple program compile
	//    and link successfully against gcc-produced code, you
	//    need to get at least this far
	// 3) get boolean operation working
	//    before we can implement any of the conditional control flow 
	//    stuff, we need to have booleans worked out.  
	// 4) control flow:
	//    we need a way to have if-elses and while loops in our language. 
	//
	// Hint: Symbols have an associated member variable called m_offset
	//    That offset can be used to figure out where in the activation 
	//    record you should look for a particuar variable

	///////////////////////////////////////////////////////////////////////////////
	//
	//  function_prologue
	//  function_epilogue
	//
	//  Together these two functions implement the callee-side of the calling
	//  convention.  A stack frame has the following layout:
	//
	//                          <- SP (before pre-call / after post-ret)
	//  high -----------------
	//       | actual arg n  |
	//       | ...           |
	//       | actual arg 1  |  <- SP (just before call / just after ret)
	//       -----------------
	//       |  Return Addr  |  <- SP (just after call / just before ret)
	//       =================
	//       | previous %ebp |
	//       -----------------
	//       | temporary 1   |
	//       | ...           |
	//       | temporary n   |  <- SP (after prologue / before epilogue)
	//  low  -----------------
	//
	//
	//			  ||		
	//			  ||
	//			 \  /
	//			  \/
	//
	//
	//  The caller is responsible for placing the actual arguments
	//  and the return address on the stack. Actually, the return address
	//  is put automatically on the stack as part of the x86 call instruction.
	//
	//  On function entry, the callee
	//
	//  (1) allocates space for the callee's temporaries on the stack
	//  
	//  (2) saves callee-saved registers (see below) - including the previous activation record pointer (%ebp)
	//
	//  (3) makes the activation record pointer (frame pointer - %ebp) point to the start of the temporary region
	//
	//  (4) possibly copies the actual arguments into the temporary variables to allow easier access
	//
	//  On function exit, the callee:
	//
	//  (1) pops the callee's activation record (temporary area) off the stack
	//  
	//  (2) restores the callee-saved registers, including the activation record of the caller (%ebp)	 
	//
	//  (3) jumps to the return address (using the x86 "ret" instruction, this automatically pops the 
	//	  return address of the stack. After the ret, remove the arguments from the stack
	//
	//	For more info on this convention, see http://unixwiz.net/techtips/win32-callconv-asm.html
	//
	//	This convention is called __cdecl
	//
	//////////////////////////////////////////////////////////////////////////////
  
  void emit_prologue(SymName *name, unsigned int size_locals, unsigned int num_args)
  {
  }

  void emit_epilogue()
  {
  }
  
  // HERE: more functions to emit code
  template<class type>
  inline int ebp_offset(type* node) {
    return -wordsize - m_st->lookup(
        node->m_attribute.m_scope, 
        node->m_symname->spelling())->get_offset();
  }

  template<class type>
  inline int ebp_arg_offset(type* node) {
    // old %ebp, %eip, %ebx, %edx
    return 4*wordsize + m_st->lookup(
        node->m_attribute.m_scope, 
        node->m_symname->spelling())->get_offset();
  }

////////////////////////////////////////////////////////////////////////////////

public:
  
  Codegen(FILE * outputfile, SymTab * st)
  {
    m_outputfile = outputfile;
    m_st = st;
    
    label_count = 0;
  }
  
  void visitProgram(Program * p)
  {
    set_text_mode();
    p->visit_children(this);
  }
  void visitFunc(Func * p)
  {
    echo(".globl %s", p->m_symname->spelling());
    echo("%s:", p->m_symname->spelling());

    // Save and update the %ebp
    echo("push %%ebp");
    echo("mov %%esp, %%ebp");

    // Save CPU registers used for temporaries
    echo("push %%ebx");
    echo("push %%edx");

    // Allocate local variables (and space for copy of arguments)
    SymScope* inner_scope =
      p->m_function_block->m_attribute.m_scope;
    int scopesize = m_st->scopesize(inner_scope);
    echo("sub $%d, %%esp", scopesize);

    // Copy arguments as local variables
    list<Param_ptr>::iterator param_iter;
    forall(param_iter, p->m_param_list) {
      Param *pip = *param_iter;
      echo("mov %d(%%ebp), %%eax", ebp_arg_offset(pip));
      echo("mov %%eax, %d(%%ebp)", ebp_offset(pip));
    }

    p->visit_children(this);

    // Release local storage
    echo("add $%d, %%esp", scopesize);
    
    // Restore saved registers
    echo("pop %%ebx");

    // Restore the old base pointer
    echo("pop %%edx");
    echo("pop %%ebp");

    // Return from the function
    echo("ret");
  }
  void visitFunction_block(Function_block * p)
  {
    p->visit_children(this);
  }
  void visitNested_block(Nested_block * p)
  {
    p->visit_children(this);
  }
  void visitAssignment(Assignment * p)
  {
    p->visit_children(this);
    echo("pop %d(%%ebp)", ebp_offset(p));
  }
  void visitArrayAssignment(ArrayAssignment * p)
  {
  }
  void visitCall(Call * p)
  {
  }
  void visitArrayCall(ArrayCall *p)
  {
  }
  void visitReturn(Return * p)
  {
    p->visit_children(this);
    echo("pop %%eax");
  }

  // control flow
  void visitIfNoElse(IfNoElse * p)
  {
    int label = new_label();
    p->m_expr->accept(this);
    echo("pop %%eax");
    echo("testl %%eax, %%eax");
    echo("jz label%d", label);
    p->m_nested_block->accept(this);
    echo("label%d:", label);
  }
  void visitIfWithElse(IfWithElse * p)
  {
    int labelElse = new_label();
    int labelEnd = new_label();
    p->m_expr->accept(this);
    echo("pop %%eax");
    echo("testl %%eax, %%eax");
    echo("jz label%d", labelElse);
    p->m_nested_block_1->accept(this);
    echo("jmp label%d", labelEnd);
    echo("label%d:", labelElse);
    p->m_nested_block_2->accept(this);
    echo("label%d:", labelEnd);
  }
  void visitWhileLoop(WhileLoop * p)
  {
  }

  // variable declarations (no code generation needed)
  void visitDecl(Decl * p)
  {
  }
  void visitParam(Param *p)
  {
  }
  void visitTInt(TInt * p)
  {
  }
  void visitTBool(TBool * p)
  {
  }
  void visitTIntArray(TIntArray * p)
  {
  }

  // comparison operations
  void visitCompare(Compare * p)
  {
  }
  void visitNoteq(Noteq * p)
  {
  }
  void visitGt(Gt * p)
  {
  }
  void visitGteq(Gteq * p)
  {
  }
  void visitLt(Lt * p)
  {
  }
  void visitLteq(Lteq * p)
  {
  }

  // arithmetic and logic operations
  void visitAnd(And * p)
  {
    p->visit_children(this);
    echo("pop %%ebx");
    echo("pop %%eax");
    echo("and %%ebx, %%eax");
    echo("push %%eax");
  }
  void visitOr(Or * p)
  {
    p->visit_children(this);
    echo("pop %%ebx");
    echo("pop %%eax");
    echo("and %%ebx, %%eax");
    echo("push %%eax");
  }
  void visitMinus(Minus * p)
  {
    p->visit_children(this);
    echo("pop %%ebx");
    echo("pop %%eax");
    // TODO: Test order might be wrong!
    echo("sub %%ebx, %%eax");
    echo("push %%eax");
  }
  void visitPlus(Plus * p)
  {
    p->visit_children(this);
    echo("pop %%ebx");
    echo("pop %%eax");
    echo("add %%ebx, %%eax");
    echo("push %%eax");
  }
  void visitTimes(Times * p)
  {
    p->visit_children(this);
    echo("pop %%ebx");
    echo("pop %%eax");
    echo("imul %%ebx"); // multiply %ebx by %eax
    echo("push %%eax");
  }
  void visitDiv(Div * p)
  {
    p->visit_children(this);
    echo("pop %%ebx");
    echo("pop %%eax");
    echo("cdq"); // sign-extend %eax into %edx
    echo("idiv %%ebx"); // divide %ebx by %edx:%eax
    echo("push %%eax");
  }
  void visitNot(Not * p)
  {
    p->visit_children(this);
    echo("pop %%eax");
    echo("not %%eax");
    echo("push %%eax");
  }
  void visitUminus(Uminus * p)
  {
    p->visit_children(this);
    echo("pop %%eax");
    echo("neg %%eax");
    echo("push %%eax");
  }
  void visitMagnitude(Magnitude * p)
  {
    p->visit_children(this);
    echo("pop %%eax");
    echo("cdq");
    echo("xor %%edx, %%eax");
    echo("sub %%edx, %%eax");
    echo("push %%eax");
  }

  // variable and constant access
  void visitIdent(Ident * p)
  {
    echo("push %d(%%ebp)", ebp_offset(p));
  }
  void visitIntLit(IntLit * p)
  {
    echo("push $%d", p->m_primitive->m_data);
  }
  void visitBoolLit(BoolLit * p)
  {
    echo("push $%d", p->m_primitive->m_data);
  }
  void visitArrayAccess(ArrayAccess * p)
  {
  }

  // special cases
  void visitSymName(SymName * p)
  {
  }
  void visitPrimitive(Primitive * p)
  {
  }
};

