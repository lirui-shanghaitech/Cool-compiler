#include <assert.h>
#include <stdio.h>
#include "emit.h"
#include "cool-tree.h"
#include "symtab.h"
#include <vector>
#include <map>
#include <algorithm>
#include <list>

enum Basicness     {Basic, NotBasic};
#define TRUE 1
#define FALSE 0

class CgenClassTable;
typedef CgenClassTable *CgenClassTableP;

class CgenNode;
typedef CgenNode *CgenNodeP;

class CgenClassTable : public SymbolTable<Symbol,CgenNode> {
private:
   int stringclasstag;
   int intclasstag;
   int boolclasstag;


// The following methods emit code for
// constants and global declarations.

   void code_global_data();
   void code_global_text();
   void code_bools(int);
   void code_select_gc();
   void code_constants();

// The following creates an inheritance graph from
// a list of classes.  The graph is implemented as
// a tree of `CgenNode', and class names are placed
// in the base class symbol table.

   void install_basic_classes();
   void install_class(CgenNodeP nd);
   void install_classes(Classes cs);
   void build_inheritance_tree();
   void set_relations(CgenNodeP nd);
public:
   //******************************* New variable definitions*******************************
   ostream& str;   
   List<CgenNode> *nds;   
   std::vector<CgenNode*>* all_classes;       // Store all the classes to this vector
   std::map<Symbol, int>* all_classes_tags;   // Store tags of all the classes
   //******************************* End variable definitions*******************************
   
   //******************************* New method definitions*******************************
   void add_method_attrib();     // First pass of inherite tree, update attibs and methods
   void gen_all_classes();       // Update the all_classes, in order to allocate tags
   void gen_all_classes_tags();  // Update the tags of all classes
   void gen_all_methods();       // Generate the all methods of one class, overrided is considered
   CgenNode* get_cgen_node(Symbol cname);

   void code_class_name_table();    // Emit the class name table.
   void code_class_object_table();  // Emit the object table
   void code_class_dispatch_table();   // Emit the class dispatch table
   void code_class_proto_object();     // Emit the prototype object
   void code_class_init();   // Emit the class object initialization   
   void code_class_methods();      // Emit code for methods   
   void init_var_offset();         // Init the offset of variables
   //******************************* End method definitions*******************************  
   CgenClassTable(Classes, ostream& str);
   void code();
   CgenNodeP root();
};

class Var_offset {
   public:
      int offset;
      char* regi;
   public:
      Var_offset(int off_set, char* reg) {offset = off_set; regi = reg;};
};

class CgenNode : public class__class {
private: 
   Basicness basic_status;                    // `Basic' if class is basic
                                              // `NotBasic' otherwise

public:
   //******************************* New variable definitions*******************************
   List<CgenNode> *children;                  // Children of class   
   CgenNodeP parentnd;                        // Parent of class
   std::vector<method_class*>* all_methods;   // All methods including, inherites and personal
   std::vector<method_class*>* methods;       // Personal methods defined in class
   std::vector<attr_class*>* attribs;         // All attributes including inherites and personal  
   std::map   <Symbol, int>* method_index_table;   // Locating the locaton of method in AR
   std::vector<CgenNode*> inherit_path;       // Store all classes along inherit path
   std::map   <Symbol, Symbol>* method_dispatch;   // class name and method name pair
   std::map   <Symbol, int>* attrib_index_table;   // Locating the location of attributes in AR
   SymbolTable<Symbol, Var_offset> *variables;     // Record the offset of one variable
   //******************************* End variable definitions*******************************
   
   //******************************* New method definitions*******************************
   void add_method_attrib();  // Add the method and attributes to vetor methods and attribs
   std::vector<CgenNode*> gen_inherit_path();   // Get the inherit path given the current class
   void gen_all_methods();    // Handle the method override, should be called after first pass
   
   void code_class_name_table(ostream& s);   // Emit the class name table.   
   void code_class_object_table(ostream& s); // Emit the object table
   void code_class_proto_object(ostream& s, std::map<Symbol, int>* all_classes_tags); // Emit the prototype object   
   void code_class_init(ostream& s);         // Emit the class object initialization
   void code_class_methods(ostream& s);      // Emit code for methods
   void init_var_offset(ostream& s);                   // Init the offset of variables   
   //******************************* End method definitions*******************************   
   CgenNode(Class_ c,
            Basicness bstatus,
            CgenClassTableP class_table);

   void add_child(CgenNodeP child);
   List<CgenNode> *get_children() { return children; }
   void set_parentnd(CgenNodeP p);
   CgenNodeP get_parentnd() { return parentnd; }
   int basic() { return (basic_status == Basic); }
};

class BoolConst 
{
 private: 
  int val;
 public:
  BoolConst(int);
  void code_def(ostream&, int boolclasstag);
  void code_ref(ostream&) const;
};


