
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

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;

//******************************* New variable definitions*******************************
int lable_number = 0;   // A global variable for lable number, need to update every time
CgenClassTable* cct = NULL;  // A global pointer, which points to cgen class table
CgenNodeP cnp = NULL;        // A global pointer, which points to cgen node
int local_offset = 1;   // Track the offset of local variable
//******************************* End variable definitions*******************************

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
      << WORD; emit_disptable_ref(Str, s);


 /***** Add dispatch information for class String ******/

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
      << WORD; emit_disptable_ref(Int, s);

 /***** Add dispatch information for class Int ******/

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
      << WORD; emit_disptable_ref(Bool, s);

 /***** Add dispatch information for class Bool ******/

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
   stringclasstag = 0 /* Change to your String class tag here */;
   intclasstag =    0 /* Change to your Int class tag here */;
   boolclasstag =   0 /* Change to your Bool class tag here */;
  
   cct = this;

   enterscope();
   if (cgen_debug) cout << "Building CgenClassTable" << endl;
   install_basic_classes();
   install_classes(classes);
   build_inheritance_tree();

   add_method_attrib();   // Update the attribs and methods of cgennode
   gen_all_methods();     // Consider the method override
   gen_all_classes();     // Update the all classes
   gen_all_classes_tags();// Update the all classes tags
   init_var_offset();     // Init the offset of attribs
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

//******************************************************************
// Auxilary code for prototype object, object table, dipatch table
// and object initilization, basically, method and attrs are extract
// from each class.
//******************************************************************

// Given a class, find all class along the inherit path
std::vector<CgenNode*> CgenNode::gen_inherit_path()
{
  CgenNodeP cnode = this;
  // Add the class to path until Object class
  while (cnode->name != No_class) {
    inherit_path.push_back(cnode);
    cnode = cnode->parentnd;
  }
  // Reverse the vector, so Object->child->child->....->current
  std::reverse(inherit_path.begin(), inherit_path.end());
  return inherit_path;
}


// Add the methods and attributes to vector given class. Here we don't handle the
// the method override, we will handle it latter.
void CgenNode::add_method_attrib()
{
  
  // Here we inherit the attributs from the father of current class
  if (parentnd && (parentnd->attribs)) {
    attribs = new std::vector<attr_class*>(parentnd->attribs->begin(), parentnd->attribs->end());
    methods = new std::vector<method_class*>();
    all_methods = new std::vector<method_class*>();
    method_index_table = new std::map   <Symbol, int>();
    method_dispatch = new std::map   <Symbol, Symbol>();
    attrib_index_table = new std::map   <Symbol, int>();
  } else
  {
    attribs = new std::vector<attr_class*>();
    methods = new std::vector<method_class*>();
    all_methods = new std::vector<method_class*>();
    method_index_table = new std::map   <Symbol, int>();
    method_dispatch = new std::map   <Symbol, Symbol>();
    attrib_index_table = new std::map   <Symbol, int>();
  }

  // Init the variable symbol table
  variables = new SymbolTable<Symbol, Var_offset>();
  
  // Add all attributes to attrib vector
  for (int i = features->first(); features->more(i); i = features->next(i))
  {
    if (features->nth(i)->is_attrib()) {attribs->push_back((attr_class*)features->nth(i));}
    if (features->nth(i)->is_method()) {methods->push_back((method_class*)features->nth(i));}
  }

  // Set the offset of each attributes
  for (int i = 0; i < attribs->size(); i++)
  {
    (*attrib_index_table)[(*attribs)[i]->name] = i;
  }

  // Iterate through its children and add all the attributes
  for (List<CgenNode>* cn = children; cn; cn = cn->tl())
  {
    cn->hd()->add_method_attrib();
  }
}

// This function based on the add_method_attrib function can handle situation such 
// that method override.
void CgenNode::gen_all_methods()
{
  // First find all of its partent, generate inherit_path
  std::vector<CgenNode*> ip = gen_inherit_path();
  for (int i=0; i< ip.size(); i++)
  {     
    // Iterate through all methods
    for (int j=0; j< ip[i]->methods->size(); j++)
    {

      if (method_index_table->find((*(ip[i]->methods))[j]->name) == method_index_table->end())
      {
        // Method are not override
        all_methods->push_back((*(ip[i]->methods))[j]);
        (*method_index_table)[(*(ip[i]->methods))[j]->name] = (*method_index_table).size()-1;
      } else
      {
        // Overrided methods, replace it.
        (*all_methods)[(*method_index_table)[(*(ip[i]->methods))[j]->name]] = (*(ip[i]->methods))[j];
      }
      (*method_dispatch)[(*(ip[i]->methods))[j]->name] = ip[i]->name;
    }
  }
}

/*----------------------------------------------------------------------------*/


// Update the methods, attribs vector of each class, through the root Object
void CgenClassTable::add_method_attrib()
{
  probe(Object)->add_method_attrib();
}

// Update the all_classes, in order to allocate tags
void CgenClassTable::gen_all_classes()
{
  all_classes = new std::vector<CgenNode*> ();
  for (List<CgenNode>* cn = nds; cn; cn = cn->tl())
  {
    all_classes->push_back(cn->hd());
  }
  // Reverse the vector to let the tag of basic class be small
  std::reverse(all_classes->begin(), all_classes->end());
}

// Update the tags of all classes
void CgenClassTable::gen_all_classes_tags()
{
  all_classes_tags = new std::map<Symbol, int>();
  int i = 0;
  for (i = 0; i < all_classes->size(); i++)
  {
    if ((*all_classes)[i]->name == Str)
    {
      stringclasstag = i;
    } else if ((*all_classes)[i]->name == Int)
    {
      intclasstag = i;
    } else if ((*all_classes)[i]->name == Bool)
    {
      boolclasstag = i;
    }
    all_classes_tags->insert(std::make_pair((*all_classes)[i]->name, i));
  }
}

// Generate the all methods of one class, overrided is considered
void CgenClassTable::gen_all_methods()
{
  for (List<CgenNode>* cn = nds; cn; cn = cn->tl())
  {
    cn->hd()->gen_all_methods();
  }
}

CgenNode* CgenClassTable::get_cgen_node(Symbol cname)
{
  return (*all_classes)[(*all_classes_tags)[cname]];
}

//******************************************************************
// Init variable offset: should generate after first pass of tree
//******************************************************************

void CgenNode::init_var_offset(ostream& s)
{
  this->variables->enterscope();
  for (int i = 0; i < attribs->size(); i++)
  {
    Symbol cna = ((*attribs)[i])->name;
//    s << "# " << cna << ": " << (*attrib_index_table)[cna] << std::endl;
    variables->addid(cna, new Var_offset((*attrib_index_table)[cna]+3, SELF));
  }
}

void CgenClassTable::init_var_offset()
{
  for (List<CgenNode>* cn = nds; cn; cn = cn->tl())
  {
    cn->hd()->init_var_offset(str);
  }
}
//******************************************************************
// Class prototype table: should generate after first pass of tree
//******************************************************************

void CgenNode::code_class_proto_object(ostream& s, std::map<Symbol, int>* all_classes_tags)
{
  s << WORD << "-1" << endl;
  emit_protobj_ref(name, s); 
  s << LABEL;
  s << WORD << (*all_classes_tags)[this->name] << endl;
  s << WORD << DEFAULT_OBJFIELDS + attribs->size() << endl;
  s << WORD; emit_disptable_ref(name, s); s << endl;

  // Iterate through all the features
  for (int i = 0; i < attribs->size(); i++) 
  {
    attr_class* attribute = (*attribs)[i];
    if (attribute->type_decl == Str)
    {
      s << WORD;
      stringtable.lookup_string("")->code_ref(s);
      s << std::endl;
    } else if (attribute->type_decl == Int)
    {
      s << WORD;
      inttable.lookup_string("0")->code_ref(s);
      s << std::endl;
    } else if (attribute->type_decl == Bool)
    {
      s << WORD;
      falsebool.code_ref(s);
      s << std::endl;
    } else
    {
      s << WORD;
      s << 0;
      s << std::endl;
    }
  }
}

void CgenClassTable::code_class_proto_object()
{
  for (List<CgenNode>* cn = nds; cn; cn = cn->tl())
  {
    cn->hd()->code_class_proto_object(str, this->all_classes_tags);
  }
}

//******************************************************************
// Class name table: should generate after first pass of tree
//******************************************************************

void CgenNode::code_class_name_table(ostream& s)
{
  s << WORD;
  StringEntry* str_entry = stringtable.lookup_string(name->get_string());
  str_entry->code_ref(s);
  s << std::endl;
}

void CgenClassTable::code_class_name_table()
{
  str << CLASSNAMETAB << LABEL;
  for (int i = 0; i < all_classes->size(); i++)
  {
    (*all_classes)[i]->code_class_name_table(str);
  }
}

//******************************************************************
// Class object table: should generate after first pass of tree
//******************************************************************

void CgenNode::code_class_object_table(ostream& s)
{
  s << WORD;
  emit_protobj_ref(name, s);
  s << std::endl;

  s << WORD;
  emit_init_ref(name, s);
  s << std::endl;
}

void CgenClassTable::code_class_object_table()
{
  str << CLASSOBJTAB << LABEL;
  for (int i = 0; i < all_classes->size(); i++)
  {
    (*all_classes)[i]->code_class_object_table(str);
  }
}

//******************************************************************
// Class dispatch table: should generate after first pass of tree
//******************************************************************

void CgenClassTable::code_class_dispatch_table()
{
  for (int i = 0; i < all_classes->size(); i++)
  {
    emit_disptable_ref((*all_classes)[i]->name, str); str << LABEL;
    CgenNodeP cn = (*all_classes)[i];
    for (int j = 0; j < cn->all_methods->size(); j++)
    {
      str << WORD;
      Symbol mname = (*(cn->all_methods))[j]->name;
      emit_method_ref((*(cn->method_dispatch))[mname], mname, str);
      str << std::endl;
    }
  }
}

//******************************************************************
// Class initializer: should generate after first pass of tree
//******************************************************************

void CgenNode::code_class_init(ostream& s)
{
  cnp = this;
  // Label it
  emit_init_ref(name, s); 
  s << LABEL;

  // Callee init
  emit_addiu(SP, SP, -3 * WORD_SIZE, s);
  emit_store(FP, 3, SP, s);
  emit_store(SELF, 2, SP, s);
  emit_store(RA, 1, SP, s);
  emit_addiu(FP, SP, WORD_SIZE, s);
  emit_move(SELF, ACC, s);

  // Init it father if it has father
  if (parentnd && parentnd->all_methods && parentnd->attribs)
  {
    s << JAL; 
    emit_init_ref(parentnd->name, s); 
    s << endl;
  }

  // Then initialize its attributes
  for (int i = 0; i < attribs->size(); i++)
  {
    attr_class* attribute = (*attribs)[i];
    if (attribute->init->type)
    {
      attribute->init->code(s);
      emit_store(ACC, 3+(*attrib_index_table)[attribute->name], SELF, s);
    }
  }

  // Callee end
  emit_move(ACC, SELF, s);
  emit_load(FP, 3, SP, s);
  emit_load(SELF, 2, SP, s);
  emit_load(RA, 1, SP, s);
  emit_addiu(SP, SP, (3) * WORD_SIZE, s);
  emit_return(s);
  s << std::endl;
}

void CgenClassTable::code_class_init()
{
  for (List<CgenNode>* cn = nds; cn; cn = cn->tl())
  {
    cn->hd()->code_class_init(str);
  }
}


//******************************************************************
// Class methods: should generate after first pass of tree
//******************************************************************

void CgenNode::code_class_methods(ostream& s)
{
  cnp = this;
  for (int i = 0; i < methods->size(); i++)
  {
    // Method label
    method_class* meth = (*methods)[i];
    emit_method_ref(this->name, meth->name, s);
    s << LABEL;

    // Callee init
    emit_addiu(SP, SP, -3 * WORD_SIZE, s);
    emit_store(FP, 3, SP, s);
    emit_store(SELF, 2, SP, s);
    emit_store(RA, 1, SP, s);
    emit_addiu(FP, SP, WORD_SIZE, s);
    emit_move(SELF, ACC, s);

    // Init the variable
    this->variables->enterscope();
    for (int i = meth->formals->first(); meth->formals->more(i); i = meth->formals->next(i)) 
    {
      variables->addid(meth->formals->nth(i)->get_name(),new Var_offset((3 + meth->formals->len() - 1 - i), FP));
    } 
    // Evaluate the variable
    meth->expr->code(s);
    this->variables->exitscope();
    // Callee end
    emit_load(FP, 3, SP, s);
    emit_load(SELF, 2, SP, s);
    emit_load(RA, 1, SP, s);
    emit_addiu(SP, SP, (3 + meth->formals->len()) * WORD_SIZE, s);
    emit_return(s);
  }
}

void CgenClassTable::code_class_methods()
{
  for (List<CgenNode>* cn = nds; cn; cn = cn->tl())
  {
    if (!cn->hd()->basic())
      cn->hd()->code_class_methods(str);
  }
}


//******************************************************************
// Code generation: should generate after first pass of tree
//******************************************************************

void CgenClassTable::code()
{
  if (cgen_debug) cout << "coding global data" << endl;
  code_global_data();

  if (cgen_debug) cout << "choosing gc" << endl;
  code_select_gc();

  if (cgen_debug) cout << "coding constants" << endl;
  code_constants();

  if (cgen_debug) cout << "coding name table" << endl;
  code_class_name_table();

  if (cgen_debug) cout << "coding prototype object" << endl;
  code_class_proto_object();

  if (cgen_debug) cout << "coding object table" << endl;
  code_class_object_table();

  if (cgen_debug) cout << "coding object table" << endl;
  code_class_dispatch_table();

//                 Add your code to emit
//                   - prototype objects
//                   - class_nameTab
//                   - dispatch tables
//

  if (cgen_debug) cout << "coding global text" << endl;
  code_global_text();

  if (cgen_debug) cout << "coding class object init" << endl;
  code_class_init();

  if (cgen_debug) cout << "coding class method generation" << endl;
  code_class_methods();

//                 Add your code to emit
//                   - object initializer
//                   - the class methods
//                   - etc...

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

void assign_class::code(ostream &s) {
  s << "\t# Evalute the expr" << std::endl;
  expr->code(s);
  s << "\t# Store the value " << name << std::endl;
  emit_store(ACC, cnp->variables->lookup(name)->offset, \
  cnp->variables->lookup(name)->regi, s);
}

void static_dispatch_class::code(ostream &s) {
  s << "\t# Static dispatch:" << std::endl;
  s << std::endl;
  s << "\t# Push the arctual parameters" << std::endl;
  for(int m = actual->first(); actual->more(m); m = actual->next(m)) 
  {
    actual->nth(m)->code(s);
    emit_push(ACC, s);
  }
  s << "\t# Evaluate the object" << std::endl;
  expr->code(s);

  s << "\t# Judge if the object is void object" << std::endl;
  emit_bne(ACC, ZERO, lable_number, s);
  s << endl;

  s << "\t# Handle the void object" << std::endl;
  s << LA << ACC << " str_const0" <<std::endl;
  emit_load_imm(T1, cnp->get_line_number(), s);
  emit_jal("_dispatch_abort", s);

  s << "\t# Jump to the method" << std::endl;
  emit_label_def(lable_number, s);
  lable_number++;
  emit_partial_load_address(T1, s);
  emit_disptable_ref(type_name, s);
  s << std::endl;
  CgenNode* cn = cct->get_cgen_node(type_name);
  int ind = (*(cn->method_index_table))[name];
  emit_load(T1, ind, T1, s);
  emit_jalr(T1, s);
  s << std::endl;

  
}

void dispatch_class::code(ostream &s) {
  s << "\t# Dispatch:" << std::endl;
  s << std::endl;
  s << "\t# Push the arctual parameters" << std::endl;
  for(int m = actual->first(); actual->more(m); m = actual->next(m)) 
  {
    actual->nth(m)->code(s);
    emit_push(ACC, s);
  }
  s << "\t# Evaluate the object" << std::endl;
  expr->code(s);

  s << "\t# Judge if the object is void object" << std::endl;
  emit_bne(ACC, ZERO, lable_number, s);
  s << endl;

  s << "\t# Handle the void object" << std::endl;
  s << LA << ACC << " str_const0" <<std::endl;
  emit_load_imm(T1, cnp->get_line_number(), s);
  emit_jal("_dispatch_abort", s);
  
  s << "\t# Jump to the method" << std::endl;
  emit_label_def(lable_number, s);
  lable_number++;
  Symbol cname = cnp->name;
  if (expr->get_type() != SELF_TYPE)
  {
    cname = expr->get_type();
  }
  s << std::endl;
  CgenNodeP cn = cct->get_cgen_node(cname);
  int ind = (*(cn->method_index_table))[name];
  emit_load(T1, 2, ACC, s);
  emit_load(T1, ind, T1, s);
  emit_jalr(T1, s);
  s << std::endl;
}

void cond_class::code(ostream &s) {
  s << "\t# condition if:" << std::endl;
  s << "\t# pred eval" << std::endl;

  pred->code(s);
  emit_load_bool(T1, BoolConst(0), s);
  int false_branch = lable_number++;
  int finish_branch = lable_number++;
  emit_beq(ACC, T1, false_branch, s);

  s << "\t# then expr eval" << std::endl;
  then_exp->code(s);
  emit_branch(finish_branch, s);

  s << "\t# else eval" << std::endl;
  emit_label_def(false_branch, s);
  else_exp->code(s);
  emit_label_def(finish_branch, s);
}

void loop_class::code(ostream &s) {
  s << "\t# Loop while" << std::endl;
  s << "\t# pred eval" << std::endl;

  int first_branch = lable_number++;
  int finish_branch = lable_number++;
  emit_label_def(first_branch, s);
  pred->code(s);
  emit_load_bool(T2, BoolConst(0), s);
  emit_beq(ACC, T2, finish_branch, s);
  body->code(s);
  emit_branch(first_branch, s);

  s << "\t# Jump out of the loop" << std::endl;
  emit_label_def(finish_branch, s);
  
  s << "\t# Return void finish the loop" << std::endl;
  emit_load_imm(ACC, 0, s);
}


CgenNodeP find_node(Symbol cn)
{
  
  if (cn == SELF_TYPE) {
    return cnp;
  }

  for(List<CgenNode> *l = cct->nds; l; l = l->tl()) {

    if (l->hd()->get_name() == cn) {
      return l->hd();
    }
  }
  return NULL;
}

void traverse_tree(CgenNode* t, std::vector<int>* ret)
{
  if (t != NULL)
  {
    List<CgenNode>* _children = t->children;
    while (_children != NULL) {
        ret->push_back((*cct->all_classes_tags)[_children->hd()->name]);
        traverse_tree(_children->hd(), ret);
        _children = _children->tl();
    }
  }
}

bool cmp(branch_class* first, branch_class* second)
{
  return first->tag >= second->tag;
}

void typcase_class::code(ostream &s) {
  s << "\t# CASE: " << std::endl;
  s << "\t# Evaluate case expr" << std::endl;
  expr->code(s);
  emit_bne(ACC, ZERO, lable_number, s);
  s << "\t# Abort case when case on void" << std::endl;
  emit_load_address(ACC, "str_const0", s);
  emit_load_imm(T1, cnp->get_line_number(), s);
  emit_jal("_case_abort2", s);

  emit_label_def(lable_number, s);
  emit_load(T2, 0, ACC, s);     // Load the tag of expr class


  // First get the vectorized cases
  std::vector<branch_class*> vec_cases;
  std::vector<int> cases_tag;

  // Define the tags
  lable_number++;
  int base_tag = lable_number;


  // Sort the cases tags so that the larger tags will be test first
  for (int j = cases->first(); cases->more(j); j = cases->next(j))
  {
    branch_class* case_temp = (branch_class*)cases->nth(j);
    int tag_t = (*(cct->all_classes_tags))[case_temp->type_decl];
    case_temp->set_tag(tag_t);
    vec_cases.push_back(case_temp);
  }
  lable_number += vec_cases.size() + 1;
  int finish_case = lable_number + vec_cases.size();

  // Sort vec_case from larger to smaller
  std::sort(vec_cases.begin(), vec_cases.end(), cmp);

  for (int j = 0; j < vec_cases.size(); j++)
  {
    cases_tag = {};
    branch_class* case_temp = vec_cases[j];
    CgenNodeP nn = find_node(case_temp->type_decl);
    cct->str <<"\t# New start " << nn->name << std::endl;
    //vec_node.push_back(nn);
    int tag_t = (*(cct->all_classes_tags))[case_temp->type_decl];
    cases_tag.push_back(tag_t);
    traverse_tree(nn, &cases_tag);
    for (int i = 0; i < cases_tag.size(); i++)
    {
      emit_load_imm(T3, cases_tag[i], s);
      emit_beq(T2, T3, base_tag + j, s);
    }
  }
  // No matching cases abort
  s << "\t# Judge if branch on none" << std::endl;
  emit_jal("_case_abort", s);
  emit_branch(finish_case, s);

  for (int j = 0; j < vec_cases.size(); j++)
  {
    branch_class* case_temp = vec_cases[j];
    emit_label_def(base_tag+j, s);
    case_temp->code(s, finish_case);
  }

  // Finish the case
  emit_label_def(finish_case, s); 
  lable_number++;
}

void branch_class::code(ostream&s, int base_num)
{
  cnp->variables->enterscope();
  cnp->variables->addid(name, new Var_offset(-local_offset, FP));
  emit_push(ACC, s);
  local_offset++;
  expr->code(s);
  emit_addiu(SP, SP, 4, s);
  cnp->variables->exitscope();
  local_offset--;
  emit_branch(base_num, s);
}

void block_class::code(ostream &s) {
  s << "\t# BLOCK: " << std::endl;
  for (int i = body->first(); body->more(i); i = body->next(i))
  {
    body->nth(i)->code(s);
  }
}

void let_class::code(ostream &s) {
  s << "\t# Let operation:" << std::endl;
  s << std::endl;
  s << "\t# Evaluate the initializer first" << std::endl;
  init->code(s);
  // Here we handle the basic types
  if (init->type == NULL) 
  {
      if (type_decl == Str) 
      {
          emit_load_string(ACC, stringtable.lookup_string(""), s);
      } else if (type_decl == Int) 
      {
          emit_load_int(ACC, inttable.lookup_string("0"), s);
      } else if (type_decl == Bool) 
      {
          emit_load_bool(ACC, BoolConst(0), s);
      }
  }
  // Enter the variable scope and store the variable to symbol table
  cnp->variables->enterscope();
  cnp->variables->addid(identifier, new Var_offset(-local_offset, FP));

  s << "\t# Push local value" << std::endl;
  emit_push(ACC, s);
  local_offset++;
  body->code(s);
  s << "\t# Pop local value" << std::endl;
  emit_addiu(SP, SP, 4, s);
  // Exit the scope
  cnp->variables->exitscope();
  local_offset--;
}

void plus_class::code(ostream &s) {
  s << "\t# Add operation:" << std::endl;
  s << std::endl;

  s << "\t# Evaluate the e1, and push onto stack" << std::endl;
  e1->code(s);
  emit_fetch_int(ACC, ACC, s);
  emit_push(ACC, s);
  local_offset++;

  s << "\t# Evaluate the e2, and copy it" << std::endl;
  e2->code(s);
  emit_jal("Object.copy", s);

  s << "\t# Pop the e1, and add them" << std::endl;
  emit_addiu(SP, SP, 4, s);
  emit_load(T1, 0, SP, s);
  emit_fetch_int(T2, ACC, s);
  emit_addu(T3, T1, T2, s);
  emit_store_int(T3, ACC, s);
  local_offset--;
}

void sub_class::code(ostream &s) {
  s << "\t# Sub operation:" << std::endl;
  s << std::endl;

  s << "\t# Evaluate the e1, and push onto stack" << std::endl;
  e1->code(s);
  emit_fetch_int(ACC, ACC, s);
  emit_push(ACC, s);
  local_offset++;

  s << "\t# Evaluate the e2, and copy it" << std::endl;
  e2->code(s);
  emit_jal("Object.copy", s);

  s << "\t# Pop the e1, and add them" << std::endl;
  emit_addiu(SP, SP, 4, s);
  emit_load(T1, 0, SP, s);
  emit_fetch_int(T2, ACC, s);
  emit_sub(T3, T1, T2, s);
  emit_store_int(T3, ACC, s);
  local_offset--;
}

void mul_class::code(ostream &s) {
  s << "\t# Mul operation:" << std::endl;
  s << std::endl;

  s << "\t# Evaluate the e1, and push onto stack" << std::endl;
  e1->code(s);
  emit_fetch_int(ACC, ACC, s);
  emit_push(ACC, s);
  local_offset++;

  s << "\t# Evaluate the e2, and copy it" << std::endl;
  e2->code(s);
  emit_jal("Object.copy", s);

  s << "\t# Pop the e1, and add them" << std::endl;
  emit_addiu(SP, SP, 4, s);
  emit_load(T1, 0, SP, s);
  emit_fetch_int(T2, ACC, s);
  emit_mul(T3, T1, T2, s);
  emit_store_int(T3, ACC, s);
  local_offset--;
}

void divide_class::code(ostream &s) {
  s << "\t# Div operation:" << std::endl;
  s << std::endl;

  s << "\t# Evaluate the e1, and push onto stack" << std::endl;
  e1->code(s);
  emit_fetch_int(ACC, ACC, s);
  emit_push(ACC, s);
  local_offset++;

  s << "\t# Evaluate the e2, and copy it" << std::endl;
  e2->code(s);
  emit_jal("Object.copy", s);

  s << "\t# Pop the e1, and add them" << std::endl;
  emit_addiu(SP, SP, 4, s);
  emit_load(T1, 0, SP, s);
  emit_fetch_int(T2, ACC, s);
  emit_div(T3, T1, T2, s);
  emit_store_int(T3, ACC, s);
  local_offset--;
}

void neg_class::code(ostream &s) {
  s << "\t# Neg operation:" << std::endl;
  s << "\t# Evaluate the e1" << std::endl;
  e1->code(s);
  emit_jal("Object.copy", s);
  emit_load(T1, 3, ACC, s);
  emit_neg(T2, T1, s);
  emit_store(T2, 3, ACC, s);
  s << std::endl;
}

void lt_class::code(ostream &s) {
  s << "\t# lt operation:" << std::endl;
  s << std::endl;

  s << "\t# Evaluate the e1, and push onto stack" << std::endl;
  e1->code(s);
  emit_fetch_int(ACC, ACC, s);
  emit_push(ACC, s);
  local_offset++;

  s << "\t# Evaluate the e2, and copy it" << std::endl;
  e2->code(s);
  emit_jal("Object.copy", s);

  s << "\t# Pop the e1, and add them" << std::endl;
  emit_addiu(SP, SP, 4, s);
  emit_load(T1, 0, SP, s);
  emit_fetch_int(T2, ACC, s);

  s << "\t# If t1 < t2" << std::endl;
  emit_load_bool(ACC, BoolConst(1), s);
  emit_blt(T1, T2, lable_number, s);
  emit_load_bool(ACC, BoolConst(0), s);
  emit_label_def(lable_number, s);
  lable_number++;
  local_offset--;
}

void eq_class::code(ostream &s) {
  s << "\t# Equal operation:" << std::endl;
  s << std::endl;

  s << "\t# Evaluate the e1, and push onto stack" << std::endl;
  e1->code(s);
  emit_push(ACC, s);
  local_offset++;

  s << "\t# Evaluate the e2, and copy it" << std::endl;
  e2->code(s);

  s << "\t# Pop the e1, and add them" << std::endl;
  emit_addiu(SP, SP, 4, s);
  emit_load(T1, 0, SP, s);
  emit_move(T2, ACC, s);
  local_offset--;
  emit_load_bool(ACC, BoolConst(1), s);
  if ((e1->type == Int || e1->type == Str || e1->type == Bool)
        && (e2->type == Int || e2->type == Str || e2->type == Bool)) {
            emit_load_bool(A1, BoolConst(0), s);
            emit_jal("equality_test", s);
            return;
  }
  emit_beq(T1, T2, lable_number, s);
  emit_load_bool(ACC, BoolConst(0), s);
  emit_label_def(lable_number, s);
  lable_number++;
}

void leq_class::code(ostream &s) {
  s << "\t# lt operation:" << std::endl;
  s << std::endl;

  s << "\t# Evaluate the e1, and push onto stack" << std::endl;
  e1->code(s);
  emit_fetch_int(ACC, ACC, s);
  emit_push(ACC, s);
  local_offset++;

  s << "\t# Evaluate the e2, and copy it" << std::endl;
  e2->code(s);
  emit_jal("Object.copy", s);

  s << "\t# Pop the e1, and add them" << std::endl;
  emit_addiu(SP, SP, 4, s);
  emit_load(T1, 0, SP, s);
  emit_fetch_int(T2, ACC, s);

  s << "\t# If t1 < t2" << std::endl;
  emit_load_bool(ACC, BoolConst(1), s);
  emit_bleq(T1, T2, lable_number, s);
  emit_load_bool(ACC, BoolConst(0), s);
  emit_label_def(lable_number, s);
  lable_number++;
  local_offset--;
}

void comp_class::code(ostream &s) {
  s << "\t# Not operation" << std::endl;
  s << "\t# Evaluate expr e1" << std::endl;
  e1->code(s);
  int first_branch = lable_number++;
  int finish_branch = lable_number++;

  emit_load_bool(T1, BoolConst(1), s);
  emit_beq(ACC, T1, first_branch, s);
  emit_load_bool(ACC, BoolConst(1), s);
  emit_branch(finish_branch, s);

  emit_label_def(first_branch, s);
  emit_load_bool(ACC, BoolConst(0), s);
  emit_label_def(finish_branch, s);
}

void int_const_class::code(ostream& s)  
{
  //
  // Need to be sure we have an IntEntry *, not an arbitrary Symbol
  //
  emit_load_int(ACC,inttable.lookup_string(token->get_string()),s);
}

void string_const_class::code(ostream& s)
{
  emit_load_string(ACC,stringtable.lookup_string(token->get_string()),s);
}

void bool_const_class::code(ostream& s)
{
  emit_load_bool(ACC, BoolConst(val), s);
}

void new__class::code(ostream &s) {
  s << "\t# New class" << std::endl;
  if (type_name == SELF_TYPE)
  {
    emit_load(T2, TAG_OFFSET, SELF, s); // Load class tag
    emit_sll(T2, T2, 3, s);             // Get protoj offset
    emit_load_address(T1, CLASSOBJTAB, s);  // Load class_objTab
    emit_addu(T3, T1, T2, s);               // Get protoj
    emit_load(ACC, 0, T3, s);               // Load protoj to acc
    emit_jal("Object.copy", s);             // Copy the object
    emit_load(T2, 1, T3, s);
    emit_jalr(T2, s);
    return;
  }
  if (type_name == Bool)
  {
    emit_load_bool(ACC, BoolConst(0), s);
    return;
  }
  emit_partial_load_address(ACC, s);
  emit_protobj_ref(type_name, s);
  s << std::endl;
  emit_jal("Object.copy", s);
  s << JAL << type_name << CLASSINIT_SUFFIX << std::endl;
}

void isvoid_class::code(ostream &s) {
  s << "\t# Is void method" << std::endl;
  e1->code(s);
  emit_move(T2, ACC, s);
  emit_load_bool(ACC, BoolConst(1), s);
  emit_beq(T2, ZERO, lable_number, s);
  emit_load_bool(ACC, BoolConst(0), s);
  emit_label_def(lable_number, s);
  lable_number++;
}

void no_expr_class::code(ostream &s) {
  emit_load_imm(ACC, 0, s);
}

void object_class::code(ostream &s) {
  s << "\t# Object class:" << std::endl;
  if (name == self)
  {
    s << "\t# Self object" << std::endl;
    emit_move(ACC, SELF, s);
  } else
  {
    s << "\t# " << name << cnp->variables->lookup(name)->offset<< std::endl;
    emit_load(ACC, cnp->variables->lookup(name)->offset,\
    cnp->variables->lookup(name)->regi, s);
  }
}



