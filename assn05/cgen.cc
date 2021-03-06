
//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `CgenClassTable::CgenClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `CgenClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"
#include "stdlib.h"


extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;

//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol 
       arg,
       arg2,
       Bool,
       concat,
       cool_abort,
       copy,
       Int,
       in_int,
       in_string,
       IO,
       length,
       Main,
       main_meth,
       No_class,
       No_type,
       Object,
       out_int,
       out_string,
       prim_slot,
       self,
       SELF_TYPE,
       Str,
       str_field,
       substr,
       type_name,
       val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
  arg         = idtable.add_string("arg");
  arg2        = idtable.add_string("arg2");
  Bool        = idtable.add_string("Bool");
  concat      = idtable.add_string("concat");
  cool_abort  = idtable.add_string("abort");
  copy        = idtable.add_string("copy");
  Int         = idtable.add_string("Int");
  in_int      = idtable.add_string("in_int");
  in_string   = idtable.add_string("in_string");
  IO          = idtable.add_string("IO");
  length      = idtable.add_string("length");
  Main        = idtable.add_string("Main");
  main_meth   = idtable.add_string("main");
//   _no_class is a symbol that can't be the name of any 
//   user-defined class.
  No_class    = idtable.add_string("_no_class");
  No_type     = idtable.add_string("_no_type");
  Object      = idtable.add_string("Object");
  out_int     = idtable.add_string("out_int");
  out_string  = idtable.add_string("out_string");
  prim_slot   = idtable.add_string("_prim_slot");
  self        = idtable.add_string("self");
  SELF_TYPE   = idtable.add_string("SELF_TYPE");
  Str         = idtable.add_string("String");
  str_field   = idtable.add_string("_str_field");
  substr      = idtable.add_string("substr");
  type_name   = idtable.add_string("type_name");
  val         = idtable.add_string("_val");
}

static char *gc_init_names[] =
  { "_NoGC_Init", "_GenGC_Init", "_ScnGC_Init" };
static char *gc_collect_names[] =
  { "_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect" };


//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

//////////////////////////////////////////////////////////
// file scope variables
/////////////////////////////////////////////////////////

// contain a menthod's class name and offset
// for dispatch table
struct methodPair {
  Symbol className;
  int offset;
  methodPair(Symbol s, int index):className(s),offset(index) {}
};


// class tag = size - 1 - index
vector<CgenNodeP> cla;

// map<class name, map<attibute name, offset >
static map<Symbol, map<Symbol, int> > attrTable;

// map<class name, map<method name, pair> >
static map<Symbol, map<Symbol, methodPair*> > dispTable;

// map<class name, vector<method name> >
// contain offset of a method
static map<Symbol, vector<Symbol> > methOrder;

// contain current arguments
static map<Symbol, int> argList;

// label index
static int labelIndex = 0;

// for new variables in let expressions
static vector<Symbol> letVars;



//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//*********************************************************

void program_class::cgen(ostream &os) 
{
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  initialize_constants();
  CgenClassTable *codegen_classtable = new CgenClassTable(classes,os);


  /* everything is done in CgenClassTalbe:code()  */



  os << "\n# end of generated code\n";
}


//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_load(char *dest_reg, int offset, char *source_reg, ostream& s)
{
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")" 
    << endl;
}

static void emit_store(char *source_reg, int offset, char *dest_reg, ostream& s)
{
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
      << endl;
}

static void emit_load_imm(char *dest_reg, int val, ostream& s)
{ s << LI << dest_reg << " " << val << endl; }

static void emit_load_address(char *dest_reg, char *address, ostream& s)
{ s << LA << dest_reg << " " << address << endl; }

static void emit_partial_load_address(char *dest_reg, ostream& s)
{ s << LA << dest_reg << " "; }

static void emit_load_bool(char *dest, const BoolConst& b, ostream& s)
{
  emit_partial_load_address(dest,s);
  b.code_ref(s);
  s << endl;
}

static void emit_load_string(char *dest, StringEntry *str, ostream& s)
{
  emit_partial_load_address(dest,s);
  str->code_ref(s);
  s << endl;
}

static void emit_load_int(char *dest, IntEntry *i, ostream& s)
{
  emit_partial_load_address(dest,s);
  i->code_ref(s);
  s << endl;
}

static void emit_move(char *dest_reg, char *source_reg, ostream& s)
{ s << MOVE << dest_reg << " " << source_reg << endl; }

static void emit_neg(char *dest, char *src1, ostream& s)
{ s << NEG << dest << " " << src1 << endl; }

static void emit_add(char *dest, char *src1, char *src2, ostream& s)
{ s << ADD << dest << " " << src1 << " " << src2 << endl; }

static void emit_addu(char *dest, char *src1, char *src2, ostream& s)
{ s << ADDU << dest << " " << src1 << " " << src2 << endl; }

static void emit_addiu(char *dest, char *src1, int imm, ostream& s)
{ s << ADDIU << dest << " " << src1 << " " << imm << endl; }

static void emit_div(char *dest, char *src1, char *src2, ostream& s)
{ s << DIV << dest << " " << src1 << " " << src2 << endl; }

static void emit_mul(char *dest, char *src1, char *src2, ostream& s)
{ s << MUL << dest << " " << src1 << " " << src2 << endl; }

static void emit_sub(char *dest, char *src1, char *src2, ostream& s)
{ s << SUB << dest << " " << src1 << " " << src2 << endl; }

static void emit_sll(char *dest, char *src1, int num, ostream& s)
{ s << SLL << dest << " " << src1 << " " << num << endl; }

// had to add this  myself
static void emit_jr (char* dest, ostream& s) {
  s << "\tjr\t" << dest << endl;
}

static void emit_jalr(char *dest, ostream& s)
{ s << JALR << "\t" << dest << endl; }

static void emit_jal(char *address,ostream &s)
{ s << JAL << address << endl; }

static void emit_return(ostream& s)
{ s << RET << endl; }

static void emit_gc_assign(ostream& s)
{ s << JAL << "_GenGC_Assign" << endl; }

static void emit_disptable_ref(Symbol sym, ostream& s)
{  s << sym << DISPTAB_SUFFIX; }

static void emit_init_ref(Symbol sym, ostream& s)
{ s << sym << CLASSINIT_SUFFIX; }

static void emit_label_ref(int l, ostream &s)
{ s << "label" << l; }

// mine
static void emit_b (int l, ostream& s) {
  s << "\tb\t";
  emit_label_ref(l,s);
  s << endl;
}

static void emit_protobj_ref(Symbol sym, ostream& s)
{ s << sym << PROTOBJ_SUFFIX; }

static void emit_method_ref(Symbol classname, Symbol methodname, ostream& s)
{ s << classname << METHOD_SEP << methodname; }

static void emit_label_def(int l, ostream &s)
{
  emit_label_ref(l,s);
  s << ":" << endl;
}

static void emit_beqz(char *source, int label, ostream &s)
{
  s << BEQZ << source << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_beq(char *src1, char *src2, int label, ostream &s)
{
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bne(char *src1, char *src2, int label, ostream &s)
{
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bleq(char *src1, char *src2, int label, ostream &s)
{
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blt(char *src1, char *src2, int label, ostream &s)
{
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blti(char *src1, int imm, int label, ostream &s)
{
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bgti(char *src1, int imm, int label, ostream &s)
{
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_branch(int l, ostream& s)
{
  s << BRANCH;
  emit_label_ref(l,s);
  s << endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(char *reg, ostream& str)
{
  emit_store(reg,0,SP,str);
  emit_addiu(SP,SP,-4,str);
}

//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
static void emit_fetch_int(char *dest, char *source, ostream& s)
{ emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(char *source, char *dest, ostream& s)
{ emit_store(source, DEFAULT_OBJFIELDS, dest, s); }


static void emit_test_collector(ostream &s)
{
  emit_push(ACC, s);
  emit_move(ACC, SP, s); // stack end
  emit_move(A1, ZERO, s); // allocate nothing
  s << JAL << gc_collect_names[cgen_Memmgr] << endl;
  emit_addiu(SP,SP,4,s);
  emit_load(ACC,0,SP,s);
}

static void emit_gc_check(char *source, ostream &s)
{
  if (source != (char*)A1) emit_move(A1, source, s);
  s << JAL << "_gc_check" << endl;
}


///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag)
{
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s  << LABEL                                             // label
      << WORD << stringclasstag << endl                                 // tag
      << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len+4)/4) << endl // size
      << WORD;


 /***** Add dispatch information for class String ******/
  s << "String_dispTab";
      s << endl;                                              // dispatch table
      s << WORD;  lensym->code_ref(s);  s << endl;            // string length
  emit_string_constant(s,str);                                // ascii string
  s << ALIGN;                                                 // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag)
{  
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,stringclasstag);
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                // label
      << WORD << intclasstag << endl                      // class tag
      << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl  // object size
      << WORD; 

 /***** Add dispatch information for class Int ******/
  s << "Int_dispTab";
      s << endl;                                          // dispatch table
      s << WORD << str << endl;                           // integer value
}


//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag)
{
  for (List<IntEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,intclasstag);
}


//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const
{
  s << BOOLCONST_PREFIX << val;
}
  
//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                  // label
      << WORD << boolclasstag << endl                       // class tag
      << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl   // object size
      << WORD;

 /***** Add dispatch information for class Bool ******/
  s << "Bool_dispTab";
      s << endl;                                            // dispatch table
      s << WORD << val << endl;                             // value (0 or 1)
}


//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_data()
{
  Symbol main    = idtable.lookup_string(MAINNAME);
  Symbol string  = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc   = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << endl;
  str << GLOBAL; emit_protobj_ref(main,str);    str << endl;
  str << GLOBAL; emit_protobj_ref(integer,str); str << endl;
  str << GLOBAL; emit_protobj_ref(string,str);  str << endl;
  str << GLOBAL; falsebool.code_ref(str);  str << endl;
  str << GLOBAL; truebool.code_ref(str);   str << endl;
  str << GLOBAL << INTTAG << endl;
  str << GLOBAL << BOOLTAG << endl;
  str << GLOBAL << STRINGTAG << endl;

  //
  // We also need to know the tag of the Int, String, and Bool classes
  // during code generation.
  //
  str << INTTAG << LABEL
      << WORD << intclasstag << endl;
  str << BOOLTAG << LABEL 
      << WORD << boolclasstag << endl;
  str << STRINGTAG << LABEL 
      << WORD << stringclasstag << endl;    
}


//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
  str << GLOBAL << HEAP_START << endl
      << HEAP_START << LABEL 
      << WORD << 0 << endl
      << "\t.text" << endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"),str);
  str << endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << endl;
}

void CgenClassTable::code_bools(int boolclasstag)
{
  falsebool.code_def(str,boolclasstag);
  truebool.code_def(str,boolclasstag);
}

void CgenClassTable::code_select_gc()
{
  //
  // Generate GC choice constants (pointers to GC functions)
  //
  str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
  str << "_MemMgr_INITIALIZER:" << endl;
  str << WORD << gc_init_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
  str << "_MemMgr_COLLECTOR:" << endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_TEST" << endl;
  str << "_MemMgr_TEST:" << endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}

// possiblely needs more code

//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done

// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void CgenClassTable::code_constants()
{
  //
  // Add constants that are required by the code generator.
  //
  stringtable.add_string("");
  inttable.add_string("0");

  stringtable.code_string_table(str,stringclasstag);
  inttable.code_string_table(str,intclasstag);
  code_bools(boolclasstag);
}


CgenClassTable::CgenClassTable(Classes classes, ostream& s) : nds(NULL) , str(s)
{
   stringclasstag = 4 /* Change to your String class tag here */;
   intclasstag =    2 /* Change to your Int class tag here */;
   boolclasstag =   3 /* Change to your Bool class tag here */;

   enterscope();
   if (cgen_debug) cout << "Building CgenClassTable" << endl;
   install_basic_classes();
   install_classes(classes);
   build_inheritance_tree();

   code();
   exitscope();
}

void CgenClassTable::install_basic_classes()
{

// The tree package uses these globals to annotate the classes built below.
  //curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

//
// A few special class names are installed in the lookup table but not
// the class list.  Thus, these classes exist, but are not part of the
// inheritance hierarchy.
// No_class serves as the parent of Object and the other special classes.
// SELF_TYPE is the self class; it cannot be redefined or inherited.
// prim_slot is a class known to the code generator.
//
  addid(No_class,
	new CgenNode(class_(No_class,No_class,nil_Features(),filename),
			    Basic,this));
  addid(SELF_TYPE,
	new CgenNode(class_(SELF_TYPE,No_class,nil_Features(),filename),
			    Basic,this));
  addid(prim_slot,
	new CgenNode(class_(prim_slot,No_class,nil_Features(),filename),
			    Basic,this));

// 
// The Object class has no parent class. Its methods are
//        cool_abort() : Object    aborts the program
//        type_name() : Str        returns a string representation of class name
//        copy() : SELF_TYPE       returns a copy of the object
//
// There is no need for method bodies in the basic classes---these
// are already built in to the runtime system.
//
  install_class(
   new CgenNode(
    class_(Object, 
	   No_class,
	   append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	   filename),
    Basic,this));

// 
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
   install_class(
    new CgenNode(
     class_(IO, 
            Object,
            append_Features(
            append_Features(
            append_Features(
            single_Features(method(out_string, single_Formals(formal(arg, Str)),
                        SELF_TYPE, no_expr())),
            single_Features(method(out_int, single_Formals(formal(arg, Int)),
                        SELF_TYPE, no_expr()))),
            single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
            single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	   filename),	    
    Basic,this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer. 
//
   install_class(
    new CgenNode(
     class_(Int, 
	    Object,
            single_Features(attr(val, prim_slot, no_expr())),
	    filename),
     Basic,this));

//
// Bool also has only the "val" slot.
//
    install_class(
     new CgenNode(
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename),
      Basic,this));

//
// The class Str has a number of slots and operations:
//       val                                  ???
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//       
   install_class(
    new CgenNode(
      class_(Str, 
	     Object,
             append_Features(
             append_Features(
             append_Features(
             append_Features(
             single_Features(attr(val, Int, no_expr())),
            single_Features(attr(str_field, prim_slot, no_expr()))),
            single_Features(method(length, nil_Formals(), Int, no_expr()))),
            single_Features(method(concat, 
				   single_Formals(formal(arg, Str)),
				   Str, 
				   no_expr()))),
	    single_Features(method(substr, 
				   append_Formals(single_Formals(formal(arg, Int)), 
						  single_Formals(formal(arg2, Int))),
				   Str, 
				   no_expr()))),
	     filename),
        Basic,this));

}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void CgenClassTable::install_class(CgenNodeP nd)
{
  Symbol name = nd->get_name();

  if (probe(name))
    {
      return;
    }

  // The class name is legal, so add it to the list of classes
  // and the symbol table.
  nds = new List<CgenNode>(nd,nds);
  addid(name,nd);
}

void CgenClassTable::install_classes(Classes cs)
{
  for(int i = cs->first(); cs->more(i); i = cs->next(i))
    install_class(new CgenNode(cs->nth(i),NotBasic,this));
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree()
{
  for(List<CgenNode> *l = nds; l; l = l->tl())
      set_relations(l->hd());
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd)
{
  CgenNode *parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n)
{
  children = new List<CgenNode>(n,children);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}


/*
 *********************************************
 * my code starts here
 *
 *********************************************
 */


/*
 * CgenClassTable::code_dispatch
 * -----------------------------------------
 * emit code for dispatch table of each class
 * each table includes all methods from its ancestors
 *
 */
void CgenClassTable::code_dispatch (CgenNode* nd, Symbol curClass) {
  if (nd->get_name() != Object) {
    code_dispatch(nd->get_parentnd(),curClass);
  }
  // emit methods
  for (int i = nd->features->first(); nd->features->more(i); i = nd->features->next(i)) {
    method_class* meth = dynamic_cast<method_class*>(nd->features->nth(i));
    if (meth != NULL) {
      map<Symbol,methodPair*> dispList = dispTable[curClass];
      // store method and offset pair
      if (dispList.find(meth->name) != dispList.end()) {
	dispList[meth->name]->className = nd->get_name();
	dispTable[curClass] = dispList;
      }else {
	int size = dispTable[curClass].size();
	methodPair* mp = new methodPair(nd->get_name(),size);
	dispTable[curClass].insert(std::pair<Symbol,methodPair*>(meth->name,mp));
	methOrder[curClass].push_back(meth->name);
      }
    }
  }
}

// generate dispatch tables for all classes
// dispTable is a file scope variabel that needs to be populated here
// methOrder contains all classes' methods layouts
void CgenClassTable::code_dispatchTab () {
  for (List<CgenNode> *l = nds; l; l = l->tl()) {
    Symbol className =  l->hd()->get_name();
    map<Symbol,methodPair*> dispList;
    vector<Symbol> mo;
    dispTable.insert(std::pair<Symbol,map<Symbol,methodPair*> >(className,dispList));
    methOrder.insert(std::pair<Symbol,vector<Symbol> >(className,mo));
    code_dispatch(l->hd(),className);
  }
  
  // emit code for class dispatch table
  for (List<CgenNode> *l = nds; l; l = l->tl()) {
    Symbol className =  l->hd()->get_name();
    map<Symbol,methodPair*> dispList = dispTable[l->hd()->get_name()];
    vector<Symbol> mo = methOrder[className];
    str << className << DISPTAB_SUFFIX << LABEL;
   
    for (int i = 0; i < mo.size(); i++) {
      Symbol methName = mo[i];
      str << WORD << dispList[methName]->className << METHOD_SEP << methName << endl;
    }
  }
}

// CgenClassTable::code_class_objTab
// emit code for class_objTab
void CgenClassTable::code_class_objTab () {
  str << CLASSOBJTAB << LABEL;
  for (int i = cla.size()- 1; i >= 0; i--) {
    str << WORD << cla[i]->get_name() << PROTOBJ_SUFFIX << endl;
    str << WORD << cla[i]->get_name() << CLASSINIT_SUFFIX << endl;
  }
}

// count total number of attributes in a class and its ancestors
int CgenClassTable::help_numOfAtt (CgenNode *nd) {
  int num = 0;
  while (nd->get_name() != Object) {
    for (int i = nd->features->first(); nd->features->more(i); i = nd->features->next(i)) {
      attr_class *at = dynamic_cast<attr_class*>(nd->features->nth(i));
      if (at != NULL) {
	num++;
      }
    }
    nd = nd->get_parentnd();
  }
  if (cgen_debug) cout << num << endl;
  return num;
}

/*
 * CgenClassTable::code_prototype_obj
 * class inherits all attributes of its ancestors
 * emit code for class prototypes
 * ------------------------------
 * prototype layout:
 * gc tag
 * class label: class tag
 * obj size
 * dispatch pointer
 * attribute#1
 * attribute#2
 * ...
 */
void CgenClassTable::code_prototype_obj (CgenNode *nd, Symbol curClass) {
  if (nd->get_name() != Object) {
    code_prototype_obj(nd->get_parentnd(),curClass);
  }

  // emmit attributes
  for (int i = nd->features->first(); nd->features->more(i); i = nd->features->next(i)) {
    attr_class *att = dynamic_cast<attr_class*>(nd->features->nth(i));
    if (att == NULL) {
      continue;
    }

    // setup default value
    if (att->type_decl == Int) {
      IntEntry *ie = inttable.lookup_string("0");
      str << WORD; ie->code_ref(str); str << endl;
    }else if (att->type_decl == Str) {
      StringEntry *se = stringtable.lookup_string("");
      str << WORD; se->code_ref(str); str << endl;
    }else if (att->type_decl == Bool) {
      str << WORD; falsebool.code_ref(str); str << endl;
    }else { 
      // default value void for non-basic classes
      str << WORD << 0 << endl;
    }

    // store attribute and offset pair
    int size = attrTable[curClass].size();
    attrTable[curClass].insert(std::pair<Symbol,int>(att->name,size));
  }
}

void CgenClassTable::code_prototype () {
  for (int i = 0; i < cla.size(); i++) {
    int class_tag = cla.size() - 1 - i;
    Symbol className = cla[i]->get_name();
    int numOfAtt =  help_numOfAtt(cla[i]);

    str << WORD << "-1" << endl;
    str << className << PROTOBJ_SUFFIX << LABEL;
    str << WORD << class_tag << endl;
    str << WORD << 3 + numOfAtt << endl; // obj size
    str << WORD << className << DISPTAB_SUFFIX << endl;
    
    // set up a new entry for current class
    map<Symbol,int> attrList;
    attrTable.insert(std::pair<Symbol,map<Symbol,int> >(className,attrList));

    code_prototype_obj(cla[i],className);
  }
} 

/*
 * CgenClassTable::code_class_nameTab()
 * contain all class tags
 * ------------------------------------
 * First, populate the array of classes as Object is the last element
 * then emit class_name table with Object class in the first place
 *
 */
void CgenClassTable::code_class_nameTab () {
  str << CLASSNAMETAB << LABEL;

  for (List<CgenNode> *l = nds; l; l = l->tl()) {
    cla.push_back(l->hd());
  }
  
  for (int i = cla.size() - 1; i >= 0; i--) {
    char* s = cla[i]->get_name()->get_string();
    StringEntry* se = stringtable.lookup_string(s);
    str << WORD; se->code_ref(str); str << endl;
  }
}


void CgenClassTable::code_class_init (CgenNode *nd) {
  emit_addiu(SP,SP,-12,str);
  emit_store(FP,3,SP,str);
  emit_store(SELF,2,SP,str);
  emit_store(RA,1,SP,str);
  emit_addiu(FP,SP,4,str); // set $fp to a new position
  emit_move(SELF,ACC,str);

  Symbol className = nd->get_name();
  // init parent class
  // char* manipulation has to be done in C-style
  if (className != Object) {
    char* parentLabel = nd->get_parentnd()->get_name()->get_string();
    char buffer[256];
    strcpy(buffer,parentLabel);
    strcat(buffer,CLASSINIT_SUFFIX);
    emit_jal(buffer, str); 
  }
  

  for (int i = nd->features->first(); nd->features->more(i); i = nd->features->next(i)) {
    attr_class *att = dynamic_cast<attr_class*>(nd->features->nth(i));
    if (att == NULL) { // not attribute
      continue;
    }

    // if it doesn't have an initializer, no need to emit code
    Expression s = att->init;
    if (s->get_type() != NULL) { 
      att->init->code(str,className);
      int offset = 3 + attrTable[className][att->name];
      emit_store(ACC,offset,SELF,str);

      // gc
      emit_addiu(A1,SELF,offset*4,str);
      emit_jal("_GenGC_Assign",str);
    }
  }
  
  emit_move(ACC,SELF,str);
  emit_load(FP,3,SP,str);
  emit_load(SELF,2,SP,str);
  emit_load(RA,1,SP,str);
  emit_addiu(SP,SP,12,str);
  emit_jr(RA,str);
}

// initialize class init function
void CgenClassTable::code_class_initializer () {
  for (int i = cla.size() - 1; i >= 0; i--) {
    str << cla[i]->get_name() << CLASSINIT_SUFFIX << LABEL;
    code_class_init(cla[i]);
  }
} 

// calculate number of formals
int method_class::numOfFormals () {
  int count = 0;
  for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
    count++;
  }
  return count;
}


// emit code for class methods
// methods for basic classes have been defined in run-time system
void CgenClassTable::code_class_method () {
  for (int i = 0; i < cla.size() - 5; i++) {
    CgenNode* nd = cla[i];
    for (int j = nd->features->first(); nd->features->more(j); j = nd->features->next(j)) {
      method_class* meth = dynamic_cast<method_class*>(nd->features->nth(j));
      if (meth == NULL) {
	continue;
      }
      // store formals offsets for quick lookup
      map<Symbol,int> argL;
      Formals fm = meth->formals;
      for (int offset = fm->first(); fm->more(offset); offset = fm->next(offset) ) {
	argL.insert(std::pair<Symbol,int>(fm->nth(offset)->get_name(),offset));
      }
      argList = argL; // use new argument table for each method

      str << nd->get_name() << METHOD_SEP << meth->name << LABEL;
      emit_addiu(SP,SP,-12,str);
      emit_store(FP,3,SP,str);
      emit_store(SELF,2,SP,str);
      emit_store(RA,1,SP,str);
      emit_addiu(FP,SP,4,str);
      emit_move(SELF,ACC,str);
      
      meth->expr->code(str,nd->get_name());

      emit_load(FP,3,SP,str);
      emit_load(SELF,2,SP,str);
      emit_load(RA,1,SP,str);
      emit_addiu(SP,SP,meth->numOfFormals() * 4 + 12,str); // number of arguments + 12
      emit_jr(RA,str);
    }
  }

}

void CgenClassTable::code()
{
  if (cgen_debug) cout << "coding global data" << endl;
  code_global_data();

  if (cgen_debug) cout << "choosing gc" << endl;
  code_select_gc();

  if (cgen_debug) cout << "coding constants" << endl;
  code_constants();
  

//                 Add your code to emit
//                   - prototype objects
//                   - class_nameTab
//                   - dispatch tables
//
  if (cgen_debug) cout<< "coding class_nameTab" << endl;
  code_class_nameTab();

  if (cgen_debug) cout<< "coding class_objTab" << endl;
  code_class_objTab();

  if (cgen_debug) cout<< "coding dispatch tables" << endl;
  code_dispatchTab();

  if (cgen_debug) cout<< "coding prototypes" << endl;
  code_prototype();

  if (cgen_debug) cout << "coding global text" << endl;
  code_global_text();

//                 Add your code to emit
//                   - object initializer
//                   - the class methods
//                   - etc...

  if (cgen_debug) cout << "coding class initializers" << endl;
  code_class_initializer();

  if (cgen_debug) cout << "coding class methods" << endl;
  code_class_method();

}


CgenNodeP CgenClassTable::root()
{
   return probe(Object);
}


///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct) :
   class__class((const class__class &) *nd),
   parentnd(NULL),
   children(NULL),
   basic_status(bstatus)
{ 
   stringtable.add_string(name->get_string());          // Add class name to string table
}


//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************


/* 
 * assgin_class::code
 * ----------------------------
 * evaluate right hand first
 * then locate the left hand variable
 */
void assign_class::code(ostream &s,Symbol curClass) {
  expr->code(s,curClass);
  int offset;
  for (int i = letVars.size() - 1; i >= 0; i--) { // if it's in let expression
    if (name == letVars[i]) {
      offset = letVars.size() - i;
      emit_store(ACC,offset,SP,s);
      return;
    }
  }
  if (argList.find(name) != argList.end()) { // if it's a passed-in argument
    offset = argList[name] + 3; 
    emit_store(ACC,offset,FP,s);
  }else {
    offset = attrTable[curClass][name] + 3; // offset + 3 
    emit_store(ACC,offset,SELF,s);

    // gc
    emit_addiu(A1,SELF,offset*4,s);
    emit_jal("_GenGC_Assign",s);
  }
}

/*
 * help_sharedDisp
 * ------------------------------------------
 * Used by both static_dispatch and dispath to emit code for
 * 1) argument evaluation and push
 * 2) method name
 * 
 */
static int help_sharedDisp (ostream &s,Symbol curClass, Expressions actual, Expression expr) {
  int length = 0;
  for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
    length++;
  }
  for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
    actual->nth(i)->code(s,curClass);
    emit_push(ACC,s);

    // argument name does not matter
    // push this in so variables defined in let can have the right offset on stack
    letVars.push_back(No_type); 
  }  
  
  expr->code(s,curClass); // load object to $a0
  emit_bne(ACC,ZERO,labelIndex,s);
  s << LA << ACC << " " << "str_const0" << endl; // load file name
  emit_load_imm(T1,1,s);
  emit_jal("_dispatch_abort",s);

  return length;
}

/*
 * static_dispatch_class::code
 * -------------------------------------
 * 1) evaluate and push arguments to stack in reverse order
 * 2) evaluate left side of '.', push it to $a0
 * 3) route to dispatch table and loacate method in it
 *
 */
void static_dispatch_class::code(ostream &s,Symbol curClass) {
  int numOfArg = help_sharedDisp(s,curClass,actual,expr);
  curClass = type_name;
 
  // prepare class dispatch label
  char dispLabel[256];
  char* className = curClass->get_string();
  strcpy(dispLabel,className);
  strcat(dispLabel,DISPTAB_SUFFIX);

  // route to dispatch table
  emit_label_def(labelIndex,s);
  emit_load_address(T1,dispLabel,s);
  emit_load(T1,dispTable[curClass][name]->offset,T1,s);
  labelIndex++;
  emit_jalr(T1,s);
  
  // after dispatch has called, resume the right offset for variables in let
  for (int i = 0; i < numOfArg; i++) {
    letVars.pop_back();
  }
}

/* dispatch_class:code
 * -------------------------
 * 1) evaluate and push arguments to stack in reverse order
 * 2) evaluate left side of '.', push it to $a0
 * 3) loacate method in dispatch table
 */
void dispatch_class::code(ostream &s,Symbol curClass) {
  int numOfArg = help_sharedDisp(s,curClass,actual,expr);
  if (expr->get_type() != SELF_TYPE) {
    curClass = expr->get_type();
  }

  // route to dispatch table, in this case, it's at 8($s0)
  emit_label_def(labelIndex,s);
  emit_load(T1,2,ACC,s);
  emit_load(T1,dispTable[curClass][name]->offset,T1,s);
  labelIndex++;
  emit_jalr(T1,s);

  // after dispatch has been called, restore the right offset for variables in let
  for (int i = 0; i < numOfArg; i++) {
    letVars.pop_back();
  }
}

// cond_class::code
void cond_class::code(ostream &s,Symbol curClass) {
  pred->code(s,curClass); // emit code for prediction
  emit_load(T1,3,ACC,s); // load value of returned Bool object
  emit_beqz(T1,labelIndex,s); // compare it to zero
  int elseLabel = labelIndex++; // save else label
  then_exp->code(s,curClass); // evaluate then expression
  int thenLabel = labelIndex++; // save then label
  emit_b(thenLabel,s);

  emit_label_def(elseLabel,s);
  else_exp->code(s,curClass); // evaluate else expression

  emit_label_def(thenLabel,s); 
}

/*
 * loop_class::code()
 * --------------------------
 * 1) set up true label for next true execution
 * 2) evaluate pre-condition
 * 3) evaluate body
 * 4) set up false label
 *
 */
void loop_class::code(ostream &s,Symbol curClass) {
  int trueLabel = labelIndex;
  labelIndex++;
  emit_label_def(trueLabel,s); // true label
  pred->code(s,curClass);
  int falseLabel = labelIndex;
  labelIndex++;
  emit_load(T1,3,ACC,s); // load boolean value of pre-condition
  emit_beqz(T1,falseLabel,s); // test boolean value
  body->code(s,curClass);
  emit_branch(trueLabel,s); // end of while loop body, jump back to true label and evaluate pre-condition
  emit_label_def(falseLabel,s);
  emit_move(ACC,ZERO,s);
}

/*
 * typecase_class::code
 * ----------------------------------
 * 1) evaluate expr, save the class tag of expr's type
 * 2) evaluate each branch:
 *    comppare the class tags to get to the right branch
 *
 */
void typcase_class::code(ostream &s,Symbol curClass) {
  expr->code(s,curClass);
  emit_move("$s1",ACC,s); // save expr object
  emit_bne(ACC,ZERO,labelIndex,s); // test if it's a void object
  emit_load_address(ACC,"str_const0",s); // load file name
  emit_load_imm(T1,1,s); 
  emit_jal("_case_abort2",s);
  emit_label_def(labelIndex,s);
  labelIndex++;

  int successLabel = labelIndex;
  labelIndex++;
  for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
    branch_class* branch_t = (branch_class*)cases->nth(i);

    // find out class tag 
    int class_tag;
    for (int i = 0; i < cla.size(); i++) {
      if (cla[i]->get_name() == branch_t->type_decl) {
	class_tag = cla.size() - 1 - i;
      }
    }

    int notEqualLabel = labelIndex;
    emit_load(T1,0,"$s1",s); // load current class tag
    emit_blti(T1,class_tag,notEqualLabel,s); // if less, jump
    emit_bgti(T1,class_tag,notEqualLabel,s); // if greater, jump
    labelIndex++;
    letVars.push_back(branch_t->name); // save new variable
    emit_push("$s1",s); // push the value to stack

    branch_t->expr->code(s,curClass);
    emit_addiu(SP,SP,4,s); // pop stack
    letVars.pop_back();
    emit_branch(successLabel,s);
    emit_label_def(notEqualLabel,s);
  }
  emit_jal("_case_abort",s);
  emit_label_def(successLabel,s);
}

// block_class::code
// no need to emit code for blocks
void block_class::code(ostream &s,Symbol curClass) {
  for (int i = body->first(); body->more(i); i = body->next(i)) {
    body->nth(i)->code(s,curClass);
  }
}

/*
 * let_class::code
 * ---------------------------------------------
 * 1) evaluate initialzation
 * 2) assgin it to the variabel in a new location
 * 3) evaluate the body
 * all new variables are saved on stack
 */
void let_class::code(ostream &s,Symbol curClass) {
  init->code(s,curClass);

  // load default value to $a0 if there's no initializer
  if (init->get_type() == NULL) {
    if (type_decl == Int) {
      emit_load_int(ACC,inttable.lookup_string("0"),s);
    }else if (type_decl == Str) {
      emit_load_string(ACC,stringtable.lookup_string(""),s);
    }else if (type_decl == Bool) {
      emit_load_bool(ACC, BoolConst(false), s);
    }else {
      emit_load_imm(ACC,0,s);
    }
  }
  emit_push(ACC,s); // push new variable to stack
  letVars.push_back(identifier);
  body->code(s,curClass);
  letVars.pop_back();
  emit_addiu(SP,SP,4,s); // pop off stack
}

/* 
 * plus_class::code
 * -------------------------------
 * 1) evaluate e1 and save it to $t1
 * 2) evaliate e2, create a new int object and load it to $t2
 * 3) add t1 and t2
 * 4) save sum to newly created object
 *
 */
void plus_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_push(ACC,s); // save e1 object to stack
  letVars.push_back(No_type); // add offset, for the sake of variable defined in let

  e2->code(s,curClass);
  emit_jal("Object.copy",s);
  emit_load(T2,3,ACC,s); 
  emit_load(T1,1,SP,s);   // retore e1 object from stack
  emit_addiu(SP,SP,4,s);  // pop stack
  letVars.pop_back(); // pop offset, for the sake of variable defined in let

  emit_load(T1,3,T1,s);   // load int value at offset 12
  emit_add(T1,T1,T2,s); 
  emit_store(T1,3,ACC,s); 
}

// sub_class::code
// similar to plus
void sub_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_push(ACC,s); // save e1 object to stack
  letVars.push_back(No_type); // add offset, for the sake of variable defined in let

  e2->code(s,curClass);
  emit_jal("Object.copy",s);
  emit_load(T2,3,ACC,s); 
  emit_load(T1,1,SP,s);   // retore e1 object from stack
  emit_addiu(SP,SP,4,s);  // pop stack
  letVars.pop_back(); // pop offset, for the sake of variable defined in let

  emit_load(T1,3,T1,s);   // load int value at offset 12
  emit_sub(T1,T1,T2,s); 
  emit_store(T1,3,ACC,s); 
}

// mul_class::code
// similar to plus
void mul_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_push(ACC,s); // save e1 object to stack
  letVars.push_back(No_type); // add offset, for the sake of variable defined in let

  e2->code(s,curClass);
  emit_jal("Object.copy",s);
  emit_load(T2,3,ACC,s); 
  emit_load(T1,1,SP,s);   // retore e1 object from stack
  emit_addiu(SP,SP,4,s);  // pop stack
  letVars.pop_back(); // pop offset, for the sake of variable defined in let

  emit_load(T1,3,T1,s);   // load int value at offset 12
  emit_mul(T1,T1,T2,s); 
  emit_store(T1,3,ACC,s); 
}

// div_class::code
// similar to plus
void divide_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_push(ACC,s); // save e1 object to stack
  letVars.push_back(No_type); // add offset, for the sake of variable defined in let

  e2->code(s,curClass);
  emit_jal("Object.copy",s);
  emit_load(T2,3,ACC,s); 
  emit_load(T1,1,SP,s);   // retore e1 object from stack
  emit_addiu(SP,SP,4,s);  // pop stack
  letVars.pop_back(); // pop offset, for the sake of variable defined in let

  emit_load(T1,3,T1,s);   // load int value at offset 12
  emit_div(T1,T1,T2,s); 
  emit_store(T1,3,ACC,s); 
}

// neg_class::code
// similar to plus
void neg_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_jal("Object.copy",s);
  emit_load(T1,3,ACC,s);
  emit_neg(T1,T1,s);
  emit_store(T1,3,ACC,s);
}

/*
 * lt_class::code
 * -------------------------------------------
 * evaluate e1, e2
 * then make comparision
 *
 */
void lt_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_push(ACC,s); // push e1 object to stack
  letVars.push_back(No_type);
  e2->code(s,curClass);
  emit_load(T2,3,ACC,s); // load e2's value to $t2
  emit_load(T1,1,SP,s); // restore e1 object from stack
  emit_addiu(SP,SP,4,s); // pop stack
  letVars.pop_back();
  emit_load(T1,3,T1,s); // load e1's value to $t1
  emit_load_address(ACC,"bool_const1",s); // load true
  emit_blt(T1,T2,labelIndex,s);
  emit_load_address(ACC,"bool_const0",s); // load false
  emit_label_def(labelIndex,s);

  labelIndex++;
}

/*
 * eq_class:code
 * -----------------------
 * 1) compare pointers addresses
 * 2) compare values if both are the same type of Int, String or Bool
 *
 */
void eq_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_push(ACC,s); // save e1 to stack
  letVars.push_back(No_type);
  e2->code(s,curClass);
  emit_move(T2,ACC,s);
  emit_load(T1,1,SP,s);
  emit_addiu(SP,SP,4,s);
  letVars.pop_back();
  Symbol eType = e1->get_type();
  if (eType == Int || eType == Str || eType == Bool) {
    emit_load_address(ACC,"bool_const1",s); // load true to $a0
    emit_load_address(A1,"bool_const0",s); // load false to $a1
    emit_jal("equality_test",s);
  }else {
    emit_load_address(ACC,"bool_const1",s); // load true to $a0
    emit_beq(T1,T2,labelIndex,s); // compare pointer addresses
    emit_load_address(ACC,"bool_const0",s);
    emit_label_def(labelIndex,s);
    labelIndex++;
  }
}

// leq_class::code
// similar to lt_class::code
void leq_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_push(ACC,s); // push e1 object to stack
  letVars.push_back(No_type);
  e2->code(s,curClass);
  emit_load(T2,3,ACC,s); // load e2's value to $t2
  emit_load(T1,1,SP,s); // restore e1 object from stack
  emit_addiu(SP,SP,4,s); // pop stack
  letVars.pop_back();
  emit_load(T1,3,T1,s); // load e1's value to $t1
  emit_load_address(ACC,"bool_const1",s); // load true
  emit_bleq(T1,T2,labelIndex,s);
  emit_load_address(ACC,"bool_const0",s); // load false
  emit_label_def(labelIndex,s);

  labelIndex++;
}

// comp_class::code, for symbol 'not'
void comp_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_load(T1,3,ACC,s); // load boolean value
  emit_load_address(ACC,"bool_const1",s); // load true
  emit_beqz(T1,labelIndex,s);
  emit_load_address(ACC,"bool_const0",s);
  emit_label_def(labelIndex,s);
  labelIndex++;
}

void int_const_class::code(ostream& s,Symbol curClass)  
{
  //
  // Need to be sure we have an IntEntry *, not an arbitrary Symbol
  //
  emit_load_int(ACC,inttable.lookup_string(token->get_string()),s);
}

void string_const_class::code(ostream& s,Symbol curClass)
{
  emit_load_string(ACC,stringtable.lookup_string(token->get_string()),s);
}

void bool_const_class::code(ostream& s,Symbol curClass)
{
  emit_load_bool(ACC, BoolConst(val), s);
}

/*
 * new__class::code
 * ------------------------------------------
 * 1) check if new is of SELF_TYPE, if it is, look it up in class_objTab
 * 2) jump to prototype object
 * 3) jump to class init
 *
 */
void new__class::code(ostream &s,Symbol curClass) {
  if (type_name == SELF_TYPE) {
    emit_load_address(T1,"class_objTab",s); // load class_objTab
    emit_load(T2,0,SELF,s); // load class tag
    emit_sll(T2,T2,3,s); // double class tag as offset, then multiply by word size, so sll by 3
    emit_addu(T1,T1,T2,s); // add offset to class_objTab to get class protoObj
    //    emit_move("$s1",T1,s); // save address of class protObj to s1
      emit_push(T1,s);

    emit_load(ACC,0,T1,s); // load proto obj
    emit_jal("Object.copy",s); 
    // emit_load(T1,1,"$s1",s); // one step forward to get class init
     emit_load(T1,1,SP,s);
     emit_load(T1,1,T1,s);
     emit_addiu(SP,SP,4,s);

    emit_jalr(T1,s);
  }else {
    char* className = type_name->get_string();
    // prepare class object prototype label
    char objProto[256];
    strcpy(objProto,className);
    strcat(objProto,PROTOBJ_SUFFIX);
    // prepare class object init label
    char objInit[256];
    strcpy(objInit,className);
    strcat(objInit,CLASSINIT_SUFFIX);

    char* objCp = "Object.copy";
    emit_load_address(ACC,objProto,s);
    emit_jal(objCp,s);
    emit_jal(objInit,s);
  }
}

/*
 * isvoid_class::code
 */
void isvoid_class::code(ostream &s,Symbol curClass) {
  e1->code(s,curClass);
  emit_move(T1,ACC,s);
  emit_load_address(ACC,"bool_const1",s); // load true
  emit_beqz(T1,labelIndex,s);
  emit_load_address(ACC,"bool_const0",s); // load false

  emit_label_def(labelIndex,s);
  labelIndex++;
}

// no_expr_class::code
void no_expr_class::code(ostream &s,Symbol curClass) {
  // no need to emit code
}

/*
 * object_class::code
 * ------------------------
 * locate identifer
 * then load its value to $a0
 */ 
void object_class::code(ostream &s,Symbol curClass) {
  if (name == self) {
    emit_move(ACC,SELF,s);
    return;
  } 

  // if it's inside of a let expression
  int offset;
  for (int i = letVars.size() - 1; i >= 0; i--) {
    if (name == letVars[i]) {
      offset = letVars.size() - i;
      emit_load(ACC,offset,SP,s);
      return;
    }
  }
  
  // if it's a passed-in argument
  if (argList.find(name) != argList.end()) {
    offset = argList.size() -1 - argList[name] + 3;
    emit_load(ACC,offset,FP,s);
  }else {
    offset = attrTable[curClass][name] + 3; // offset + 3 
    emit_load(ACC,offset,SELF,s);
  }
}


