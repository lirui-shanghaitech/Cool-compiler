#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

// Include some stl libs
#include <vector>
#include <map>
#include <list>
#include <set>
#include <algorithm>

#define TRUE 1
#define FALSE 0


class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  ostream& error_stream;

public:
  std::map<Symbol, Class_> classes_table;         // Using this map structure to record all classes by their name
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Class_ c, tree_node *t);
  ostream& semant_error(Symbol filename, tree_node *t);
  void install_basic_classes();

  // TODO: Add your own functions here.
  void init_classes_table(Classes classes);       // Install all the classes into class table
  void init_method_table();                       // Init mehtod table based on their scope
  void check_inherit(Classes classes);            // Check the class inheritance, cycle/multi-definition etc.
  void check_method_override();                   // Check the illegal override of functions when inheritance happens
  void check_classes_type(Classes classes);                        // Check the attributes type
  std::vector<Symbol> inherit_path(Symbol type);  // Find all ancestors of type 
  bool conform(Symbol f_type, Symbol s_type);     // If the child is a valid type given ancestor type
  Symbol find_lca(Symbol f_type, Symbol s_type);  // Find the least common ancestor of two types
};


#endif
