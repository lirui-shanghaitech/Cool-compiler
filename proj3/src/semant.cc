

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
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
/* Initializing some gobal variables */
static Class_ current_class = NULL;


/* Initializing some global tables: */
static ClassTable* classtable;                      // Class table
static SymbolTable<Symbol, Symbol> attribute_table; // Attribute table

/* Define the method tables as vector of maps, where each element of 
 * vector represents a scope, for each scope its method is stored in the
 * map data structure.    */
typedef std::vector<method_class*>  Methods;
typedef std::map<Class_, Methods> Method_table;
static Method_table method_table;

void check_feature_type(Feature feature, Class_ cclass);
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



ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {

    /* Fill this in */

}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
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
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
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
	       filename);

    // Install all basic classes into the global class table
    classes_table.insert(std::make_pair(Object, Object_class));
    classes_table.insert(std::make_pair(IO, IO_class));
    classes_table.insert(std::make_pair(Int, Int_class));
    classes_table.insert(std::make_pair(Bool, Bool_class));
    classes_table.insert(std::make_pair(Str, Str_class));
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// TODO: Error message
/////////////////////////////////////////////////////////
ostream& ClassTable::semant_error(Class_ c,tree_node *t)
{                                                             
    return semant_error(c->get_filename(),t);
}  

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 

////////////////////////////////////////////////////////
// TODO: Initialize the method/class table
///////////////////////////////////////////////////////

/* Build the inheritance graph for scratch which installing all classes into 
 * the classes table, some errors are also checked such as self type, redefinition
 * ,inherite from int etc.  */
void ClassTable::init_classes_table(Classes classes)
{
    // Build the inheritance graph by installing classed into classes_table
    for (int i = classes->first(); classes->more(i); i = classes->next(i))
    {
        Class_ cur_class = classes->nth(i);
        // SELF_TYPE checking where class name can not be
        if (cur_class->get_name() == SELF_TYPE)
        {
            semant_error(cur_class) << "Class name can not be SELF TYPE. \n";
            return;
        }
        // Class can not inherit from int, bool, str and self type
        else if (cur_class->get_parent() == Int || cur_class->get_parent() == Str ||\
        cur_class->get_parent() == Bool || cur_class->get_parent() == SELF_TYPE)
        {
            semant_error(cur_class) << "Can not inherit from bool, int, str. \n";
            return;
        }
        // Class can not be redefined
        else if (classes_table.find(cur_class->get_name()) != classes_table.end())
        {
            semant_error(cur_class) << "Class " << cur_class->get_name() << " is multi defined. \n";
            return;
        }
        // If no error, insert current class to class table
        else
        {
            classes_table.insert(std::make_pair(cur_class->get_name(), cur_class));
        }
        // std::cout << cur_class->get_name() << std::endl;
    }

    // Finally we check that if main is defined.
    if ((classes_table.find(Main) == classes_table.end()))
    {
        semant_error() << "Class Main is not defined. \n";
        return;
    }
}

/* Init the method table and check the illegal in-class overriding */
void ClassTable::init_method_table()
{
    // Iterate over all classes, install the method table
    std::map<Symbol, Class_>::iterator it;
    for (it = classes_table.begin(); it != classes_table.end(); ++it)
    {
        Symbol cname = it->first;
        Features cfeatures = classes_table[cname]->get_features();
        Methods methods;

        // Find all methods in one class and find the override of methods
        std::set<Symbol> used_names;
        for (int i = cfeatures->first(); cfeatures->more(i); i = cfeatures->next(i))
        {
            if (cfeatures->nth(i)->is_method())
            {
                if (used_names.find(cfeatures->nth(i)->get_name()) == used_names.end())
                {
                    methods.push_back(static_cast<method_class*>(cfeatures->nth(i)));
                    used_names.insert(cfeatures->nth(i)->get_name());
                } else
                {
                    semant_error(it->second) << "Override of methods " << std::endl;
                }
                
            }
                
        }
        // Update the method table
        method_table.insert(std::make_pair(it->second, methods));
    }
}

///////////////////////////////////////////////////////
// TODO: Inheritance checking, intheritance path
///////////////////////////////////////////////////////

/* Check the cycle of inheritance graph */
void ClassTable::check_inherit(Classes classes)
{
    // Check if the parent of child is exist
    for (int i = classes->first(); classes->more(i); i = classes->next(i))
    {
        if (classes_table.find(classes->nth(i)->get_parent()) == classes_table.end())
        {
            semant_error(classes->nth(i)) << "Can not find father " << std::endl;
            return;
        }
    }

    // Check if the inheritance graph has cycles
    for (int i = classes->first(); classes->more(i); i = classes->next(i))
    {
        current_class = classes->nth(i);
        if (current_class->get_name() == Object) continue;

        Symbol cname = current_class->get_name();
        Symbol pname = current_class->get_parent();


        // Find the cycle for every type, except object type since it has no father
        while(pname != Object)
        {
            // Find a cycle here
            if (pname == cname)
            {
                semant_error(current_class) << "Cycle in inheritance graph " << std::endl;
                return;
            }
            // Update current class
            current_class = classes_table[pname];
            cname = current_class->get_name();
            pname = current_class->get_parent();
        }

    }
}

/* Find all ancestors given child along the inherit tree */
// | Child | child | ..... | Ancestor | .... | Object |
std::vector<Symbol> ClassTable::inherit_path(Symbol type)
{
    // First consider the self type where type equal to current class's name
    if (type == SELF_TYPE)
        type = current_class->get_name();

    // Check if the type is defined
    if (classes_table.find(type) == classes_table.end())
        semant_error(current_class) << type << " not found." << std::endl;

    // Update the inheritance path
    std::vector<Symbol> inherit_path;
    for (; type != No_class; type = classes_table[type]->get_parent())
    {
        inherit_path.push_back(type);
    }
    return inherit_path;
}

/* Find the least common ancestor given two classes */
Symbol ClassTable::find_lca(Symbol f_type, Symbol s_type)
{
    // Get the inherite path given two types
    Symbol common_ancestor = Object;
    std::vector<Symbol> p1 = inherit_path(f_type);
    std::vector<Symbol> p2 = inherit_path(s_type);
    std::vector<Symbol>::reverse_iterator it1 = p1.rbegin();
    std::vector<Symbol>::reverse_iterator it2 = p2.rbegin();

    // Check from object class to child, so stop when type different
    for (; it1 != p1.rend() && it2 != p2.rend(); it1++,it2++)
    {
        if (*it1 == *it2)
        {
            common_ancestor = *it2;
            continue;
        }
        break;
    }
    return common_ancestor;
}

///////////////////////////////////////////////////////
// TODO: Check the method table for illegal override
///////////////////////////////////////////////////////

/* Look up a method for a particular class return it if found */
static method_class* method_look_up (Methods methods, Symbol name)
{
    for (std::vector<method_class*>::iterator it  = methods.begin(); it != methods.end(); ++it)
    {
        if ((*it)->get_name() == name)
        {
            return *it;
        }
    }
    return NULL;
}

/* Check the method override among different classes. Formal type and formal numbers are checked */
void ClassTable::check_method_override()
{
    // Iterate through all the class
    for (std::map<Symbol, Class_>::iterator it=classes_table.begin(); it!=classes_table.end(); ++it)
    {
        current_class = it->second;
        Symbol cname = it->first;
        Features cfeatures = current_class->get_features();
        // Get all the ancestors
        std::vector<Symbol> apath = inherit_path(cname);

        // Iterate through all the features for one class, only consider the method here
        for (int i = cfeatures->first(); cfeatures->more(i); i = cfeatures->next(i))
        {
            // Only consider the method
            Feature cfeature = cfeatures->nth(i);
            if (cfeature->is_method())
            {   
                // First get all the formals of current methos
                Formals cformals = ((method_class*)cfeature)->get_formals();
                
                // Iterate through all its ancetors, check the parameter types
                for (std::vector<Symbol>::iterator ait = apath.begin(); ait != apath.end(); ++ait)
                {
                    Methods methods = method_table[classes_table[(*ait)]];
                    method_class* cmethod = method_look_up(methods, cfeature->get_name());

                    // If we found a method
                    if (cmethod == NULL)
                    {
                        continue;
                    } else
                    {
                        // Extract the ancestor's formals
                        Formals aformals = cmethod->get_formals();

                        // Now we check if formals of these two methods are consistanted
                        // First check if the len of formals are same
                        int m1 = cformals->first();
                        int m2 = aformals->first();
                        int c1 = 0, c2 = 0;
                        for (; cformals->more(m1); m1 = cformals->next(m1))
                        {
                            c1 = c1 + 1;
                        }
                        for (; aformals->more(m2); m2 = aformals->next(m2))
                        {
                            c2 = c2 + 1;
                        }
                        if (c1 != c2)
                        {
                            semant_error(current_class)<< "Method override: len not match" << std::endl;
                        }

                        // Then we check the type of each parameters is same
                        m1 = cformals->first();
                        m2 = aformals->first();
                        for (; cformals->more(m1) && aformals->more(m2); m1 = cformals->next(m1), m2 = aformals->next(m2))
                        {
                            if (aformals->nth(m2)->get_type() != cformals->nth(m1)->get_type())
                            {
                                semant_error(current_class) << "Method override: formal type not match " << std::endl;
                            }
                        }

                    }
                    
                }
            }
        }
    }
}


///////////////////////////////////////////////////////
// TODO: Check the all the attri, update attri table
///////////////////////////////////////////////////////

/* Add an attribute to attribute table, and check the multi-definition 
 * of one attribute. Note that, the self can not be the name of an attribute */
static void add_to_attritable(Feature feature, Class_ cla)
{
    // Only add attribute to attribute table
    if (feature->is_attribute())
    {
        // Check self and multiple definitions
        if (feature->get_name() == self || attribute_table.lookup(feature->get_name()) != NULL)
        {
            classtable->semant_error(cla) << "Self in attribute or multi-definition of one attribute" << std::endl;
        }
        // Add to attribute table
        attribute_table.addid(feature->get_name(), new Symbol(feature->get_type()));
    }
}

/* Set up the attribute table for final type checking. All attributes of current class and its
 * ancestors' attribute will be added to table in an ancestor->children order, since when check 
 * types, some attribute may define in the ancestor. */
static void set_up_attritable(Class_ cclass)
{
    // Get the inheritance path in an children->ancestor order
    std::vector<Symbol> ipath = classtable->inherit_path(cclass->get_name());

    // Iterate through all ancestors add attribute to attribute table
    for (std::vector<Symbol>::reverse_iterator it=ipath.rbegin(); it != ipath.rend(); it++)
    {
        Class_ cla = classtable->classes_table[*it];
        Features cfeature = cla->get_features();
        attribute_table.enterscope();

        // Iterate through all the features 
        for (int i = cfeature->first(); cfeature->more(i); i = cfeature->next(i))
        {
            add_to_attritable(cfeature->nth(i), cla);
        }
    }
    
}

/* Check one class's attribute, this function should be called just after the set_up_attritable */
static void check_one_class_type(Class_ cclass)
{
    // Iterate through all features, checking their type. Actually, this is the entry point to the final type checking
    Features cfeatures = cclass->get_features();
    for (int i = cfeatures->first(); cfeatures->more(i); i = cfeatures->next(i))
    {
        current_class = cclass;
        check_feature_type(cfeatures->nth(i), cclass);
    }
}

/* After checking one class's type, we need to exit scope of attribute table, so as to prepare 
 * type checking for the subsequence classes */
static void exit_scope_attritable(Class_ cclass)
{
    // Get the inheritance path in an children->ancestor order
    std::vector<Symbol> ipath = classtable->inherit_path(cclass->get_name());

    // Exit the scope
    for (std::vector<Symbol>::reverse_iterator it=ipath.rbegin(); it != ipath.rend(); it++)
    {
        attribute_table.exitscope();
    }
}

/* Check type of each class */
void ClassTable::check_classes_type(Classes classes)
{
    // Iterate through all classes
    for (int i = classes->first(); classes->more(i); i = classes->next(i))
    {
        Class_ cclass = classes->nth(i);
        set_up_attritable(cclass);
        check_one_class_type(cclass);
        exit_scope_attritable(cclass);
    }
}


///////////////////////////////////////////////////////
// TODO: Feature type checking functions
///////////////////////////////////////////////////////

/*Check whether descendant_type conforms to  ancestor_type*/
bool ClassTable::conform(Symbol ancestor_type, Symbol descendant_type)
{
    //When ancestor_type is SELF_TYPE, only SELF_TYPE conforms to SELF_TYPE.
    if (ancestor_type == SELF_TYPE && descendant_type == SELF_TYPE) 
        return true;

    //When descendant_type is SELF_TYPE and the current_class is C, C must conform to the ancestor_type.
    if (descendant_type == SELF_TYPE) 
        descendant_type = current_class->get_name();
    
    for (; descendant_type != No_class; descendant_type = classtable->classes_table.find(descendant_type)->second->get_parent()) {
        if (ancestor_type == descendant_type) {
            return true;
        }
    }
    return false;
}

/*Check the feature type*/
void check_feature_type(Feature feature, Class_ c)
{
    //Method
    if(feature->is_method()){
        ((method_class*)feature)->check_method_feature_type();
    }
    //Attribute
    else
    {
        ((attr_class*)feature)->check_attr_feature_type();
    }
    
}

/*Check the method type*/
void method_class::check_method_feature_type()
{
    //Enter the scope of definition of features.
    attribute_table.enterscope();
    //Record the current method's formals to avoid redefinition.
    std::set<Symbol> used_names;
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        Formal formal = formals->nth(i); 
        if (used_names.find(formal->get_name()) != used_names.end()) {
            classtable->semant_error(current_class,this) << "Formal: " << formal->get_name() << " is redefined.\n";
        } 
        else
            used_names.insert(formal->get_name());
        if (formal->get_name() == self)
            classtable->semant_error(current_class,this) << "'self' can be a formal.\n";
        else if (classtable->classes_table.find(formal->get_type()) == classtable->classes_table.end())
            classtable->semant_error(current_class,this) << "The type of the formal is not defined.\n";  
        //Add the formals to attribute_table and the expression type checking will use the variables in the attribute_table.
        else
            attribute_table.addid(formal->get_name(), new Symbol(formal->get_type()));
    }
    if (return_type != SELF_TYPE && classtable->classes_table.find(return_type) == classtable->classes_table.end())
         classtable->semant_error(current_class,this) << "The type of the return is not defined.\n"; 
    Symbol expr_type = expr->check_expr_type();
    if (classtable->conform(return_type, expr_type) == false) 
        classtable->semant_error(current_class,this) << "The expression type dose not conform to the return type.\n " ;
    //Exit the scope of definition of features.
    attribute_table.exitscope();
    return;   
}

/*Check the attribute type*/
void attr_class::check_attr_feature_type()
{
    Symbol init_type = init->check_expr_type();
    if(init_type != No_type && classtable->conform(type_decl, init_type)==false)
        classtable->semant_error(current_class, this) << "The expression type dose not conform to init type.\n";
    return;
}


/* The function is to check one branch of Case */
Symbol branch_class::check_branch_type()
{
    attribute_table.enterscope();
    attribute_table.addid(name, new Symbol(type_decl));
    Symbol type = expr->check_expr_type();
    attribute_table.exitscope();
    return type;
}


/*Check the assignment type*/
Symbol assign_class::check_expr_type()
{
    //The assigned variable must be declared.
    if (attribute_table.lookup(name) == NULL) {
        classtable->semant_error(current_class,this) << "Can not assign an undeclared variable.\n";
        type = Object;
        return Object;
    }
    //The expression type must conforms to the assigned variable.
    Symbol expr_type = expr->check_expr_type();
    if (classtable->conform(*attribute_table.lookup(name), expr_type) == false) {
        classtable->semant_error(current_class,this) << "The expression type dose not conform to the assigned variable's type.\n ";
        type = Object;
        return Object;
    }
    type = expr_type;
    return expr_type;
}

/*Check the static dispatch type*/
Symbol static_dispatch_class::check_expr_type()
{
    //The expresion type e0 must conform to the @T.
    Symbol expr_type = expr->check_expr_type();
    if (classtable->conform(type_name, expr_type) == false) {
        classtable->semant_error(current_class,this) << "The expression type dose not conform to the static dispatch class type.\n ";
        type = Object;
        return Object;
    }

    //Find the static dispatch method.
    method_class* method = NULL;
    Symbol type_ = (type_name == SELF_TYPE? current_class->get_name():type_name);
    int flag = 0;
    for (; type_ != No_class; type_ = classtable->classes_table.find(type_)->second->get_parent()) {
        Class_ cla = classtable->classes_table.find(type_)->second;
        Methods methods = method_table[cla];
        for (size_t i = 0; i < methods.size(); i++){
            if (methods[i]->get_name() == name) {
                method = methods[i];
                flag = 1;
                break;
            }
        }
        if (flag)
            break;
    }
    if (method == NULL) {
        classtable->semant_error(current_class,this) << "Can not find method.\n";
        type = Object;
        return Object;
    }

    //Check every actual type whether conforms to formal type.
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        Symbol actual_type = actual->nth(i)->check_expr_type();
        Symbol formal_type = method->get_formals()->nth(i)->get_type();
        if (classtable->conform(formal_type, actual_type) == false) {
            classtable->semant_error(current_class,this) << "The actual type of method parameter dose not conform to the formal type.\n ";
            type = Object;
            return Object;
        }        
    }
    type = (method->get_type() == SELF_TYPE? expr_type:method->get_type());
    return type;
}

/*Check the dispatch type*/
Symbol dispatch_class::check_expr_type()
{
    //Find the dispatch method.
    Symbol expr_type = expr->check_expr_type();
    method_class* method = NULL;
    Symbol type_ = (expr_type == SELF_TYPE? current_class->get_name():expr_type);
    int flag = 0;
    for (; type_ != No_class; type_ = classtable->classes_table.find(type_)->second->get_parent()) {
        Class_ cla = classtable->classes_table.find(type_)->second;
        Methods methods = method_table[cla];
        for (size_t i = 0; i < methods.size(); i++){
            if (methods[i]->get_name() == name) {
                method = methods[i];
                flag = 1;
                break;
            }
        }
        if (flag)
            break;
    }
    if (method == NULL) {
        classtable->semant_error(current_class,this) << "Can not find method.\n";
        type = Object;
        return Object;
    }

    //Check every actual type whether conforms to formal type.
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        Symbol actual_type = actual->nth(i)->check_expr_type();
        Symbol formal_type = method->get_formals()->nth(i)->get_type();
        if (classtable->conform(formal_type, actual_type) == false) {
            classtable->semant_error(current_class,this) << "The actual type of method parameter dose not conform to the formal type.\n ";
            type = Object;
            return Object;
        }        
    }

    type = (method->get_type() == SELF_TYPE? expr_type:method->get_type());
    return type;
}

/*Check the If type*/
Symbol cond_class::check_expr_type()
{
    //Check the pred is Bool type.
    if (pred->check_expr_type() != Bool) {
        classtable->semant_error(current_class,this) << "Type of pred is not Bool.\n ";
        type = Object;
        return Object;
    }

    Symbol then_type = then_exp->check_expr_type();
    Symbol else_type = else_exp->check_expr_type();

    //The return type is the union of then_type and else_type.
    type = classtable->find_lca(then_type, else_type);

    return type;
}

/*Check the While type*/
Symbol loop_class::check_expr_type()
{
    //Check the pred is Bool type.
    if (pred->check_expr_type() != Bool) {
        classtable->semant_error(current_class,this) << "Type of pred is not Bool.\n ";
        type = Object;
        return Object;
    }
    body->check_expr_type();
    type = Object;
    return Object;
}

/*Check the Case type*/
Symbol typcase_class::check_expr_type()
{
    Symbol expr_type = expr->check_expr_type();
    std::set<Symbol> cases_type_decls;
    std::set<Symbol> branch_types;
    //The variables declared on each branch of a case must all have distinct types.
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
        branch_class *branch = (branch_class*)cases->nth(i);
        if (cases_type_decls.find(branch->get_type()) != cases_type_decls.end()){
            classtable->semant_error(current_class,this) << "The variables declared on each branch of a case must all have distinct types.\n ";
            type = Object;
            return Object;
        }   
        else
            cases_type_decls.insert(branch->get_type());

        Symbol branch_type = branch->check_branch_type();
        branch_types.insert(branch_type);
    }

    //The type of the entire case is the join of the types of its branches.
    type = *branch_types.begin();
    std::set<Symbol>::iterator it; 
    for(it = branch_types.begin()++; it != branch_types.end(); it++){ 
        type = classtable->find_lca(type, *it);
    }
    return type;
}

/*Check the Block type*/
Symbol block_class::check_expr_type()
{
    //Check every expression in block.
    for (int i = body->first(); body->more(i); i = body->next(i)) {
        type = body->nth(i)->check_expr_type();
    }
    return type;
}

/*Check the Let type*/
Symbol let_class::check_expr_type()
{
    attribute_table.enterscope();
    
    if (type_decl != SELF_TYPE && classtable->classes_table.find(type_decl) == classtable->classes_table.end()){
        classtable->semant_error(current_class,this) << "The type of the identifier is not defined.\n";  
        type = Object;
        return Object;
    }

    if (identifier == self){
        classtable->semant_error(current_class,this) << "The identifier can not be self.\n";  
        type = Object;
        return Object;
    }
    
    //The Let with initiation.
    Symbol init_type = init->check_expr_type();
    if (init_type != No_type  && classtable->conform(type_decl, init_type) == false) {
        classtable->semant_error(current_class,this) << "The type_decl dose not conform to the init_type.\n " ;
        type = Object;
        return Object;
    }        
    attribute_table.addid(identifier, new Symbol(type_decl));
    type = body->check_expr_type();
    attribute_table.exitscope();

    return type;
}

/*Check the "+" type*/
Symbol plus_class::check_expr_type()
{
    Symbol type1 = e1->check_expr_type();
    Symbol type2 = e2->check_expr_type();
    //The exprresion  type must be Int.
    if (type1 != Int || type2 != Int){
        classtable->semant_error(current_class,this) << "The operand of '+' is not INT.\n";
        type = Object;
        return Object;
    }
    type = Int;
    return Int;
}

/*Check the "-" type*/
Symbol sub_class::check_expr_type()
{
    Symbol type1 = e1->check_expr_type();
    Symbol type2 = e2->check_expr_type();
    //The exprresion  type must be Int.
    if (type1 != Int || type2 != Int){
        classtable->semant_error(current_class,this) << "The operand of '-' is not INT.\n";
        type = Object;
        return Object;
    }
    type = Int;
    return Int;
}

/*Check the "*" type*/
Symbol mul_class::check_expr_type()
{
    Symbol type1 = e1->check_expr_type();
    Symbol type2 = e2->check_expr_type();
    //The exprresion  type must be Int
    if (type1 != Int || type2 != Int){
        classtable->semant_error(current_class,this) << "The operand of '*' is not INT.\n";
        type = Object;
        return Object;
    }
    type = Int;
    return Int;
}

/*Check the "/" type*/
Symbol divide_class::check_expr_type()
{
    Symbol type1 = e1->check_expr_type();
    Symbol type2 = e2->check_expr_type();
    //The exprresion  type must be Int
    if (type1 != Int || type2 != Int){
        classtable->semant_error(current_class,this) << "The operand of '/' is not INT.\n";
        type = Object;
        return Object;
    }
    type = Int;
    return Int;
}

/*Check the "~" type*/
Symbol neg_class::check_expr_type()
{
    Symbol type1 = e1->check_expr_type();
    //The exprresion  type must be Int
    if (type1 != Int){
        classtable->semant_error(current_class,this) << "The operand of '~' is not INT.\n";
        type = Object;
        return Object;
    }
    type = Int;
    return Int;
}

/*Check the "<" type*/
Symbol lt_class::check_expr_type()
{
    Symbol type1 = e1->check_expr_type();
    Symbol type2 = e2->check_expr_type();
    //The exprresion  type must be Int
    if (type1 != Int || type2 != Int){
        classtable->semant_error(current_class,this) << "The operand of '<' is not INT.\n";
        type = Object;
        return Object;
    }
    type = Bool;
    return Bool;
}

/*Check the "=" type*/
Symbol eq_class::check_expr_type()
{
    Symbol type1 = e1->check_expr_type();
    Symbol type2 = e2->check_expr_type();
    //any types may be freely compared except Int, String and Bool, which may only be compared with objects of the same type.
    if ((type1 == Int || type1 == Bool || type1 == Str || type2 == Int || type2 == Bool || type2 == Str))
        if (type1 != type2) {
            classtable->semant_error(current_class,this) << " '=' has two different types for Int,Bool,Str.\n";
            type = Object;
            return Object;
        }
        else
        {
            type = Bool;
            return Bool;
        }
    else{
            type = Bool;
            return Bool;
        }
}

/*Check the "<=" type*/
Symbol leq_class::check_expr_type()
{
    Symbol type1 = e1->check_expr_type();
    Symbol type2 = e2->check_expr_type();
    //The exprresion  type must be Int
    if (type1 != Int || type2 != Int){
        classtable->semant_error(current_class,this) << "The operand of '<=' is not INT.\n";
        type = Object;
        return Object;
    }
    type = Bool;
    return Bool;
}

/*Check the "Not" type*/
Symbol comp_class::check_expr_type()
{
    Symbol type1 = e1->check_expr_type();
    //The exprresion  type must be Bool.
    if (type1 != Bool){
        classtable->semant_error(current_class,this) << "The expression type for 'Not' is not Bool.\n";
        type = Object;
        return Object;
    }
    type = Bool;
    return Bool;
}

/*Check the "Int" type*/
Symbol int_const_class::check_expr_type()
{
    type = Int;
    return Int;
}

/*Check the "Bool" type*/
Symbol bool_const_class::check_expr_type()
{
    type = Bool;
    return Bool;
}

/*Check the "Str" type*/
Symbol string_const_class::check_expr_type()
{
    type = Str;
    return Str;
}

/*Check the "New" type*/
Symbol new__class::check_expr_type()
{
    if (type_name != SELF_TYPE && classtable->classes_table.find(type_name) == classtable->classes_table.end()) {
        classtable->semant_error(current_class,this) << "The type for 'New' is not defined.\n";
        type = Object;
        return Object;
    }
    type = type_name;
    return type_name;
}

/*Check the "isvoid" type*/
Symbol isvoid_class::check_expr_type()
{
    e1->check_expr_type();
    type = Bool;
    return Bool;
}

/*Check the "no_expr" type*/
Symbol no_expr_class::check_expr_type()
{
    return No_type;
}

/*Check the "object" type*/
Symbol object_class::check_expr_type()
{
    //The type of self Object is SELF_TYPE
    if (name == self) {
        type = SELF_TYPE;
        return SELF_TYPE;
    }
    //Find the object in the object environment.
    if (attribute_table.lookup(name)){ 
        type = *attribute_table.lookup(name);
        return *attribute_table.lookup(name);
    }
    else {
        classtable->semant_error(current_class,this) << "The object is not decleared.\n";
        type = Object;
        return Object;;
    }
}


/////////////////////////////////////////////////////////////
// TODO: Entry point for type checking
/////////////////////////////////////////////////////////////
/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    classtable = new ClassTable(classes);
    classtable->install_basic_classes();
    classtable->init_classes_table(classes);
    classtable->check_inherit(classes);

    /* some semantic analysis code may go here */
    if (classtable->errors()) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
    classtable->init_method_table();
    classtable->check_method_override();
    classtable->check_classes_type(classes);
     /* some semantic analysis code may go here */
    if (classtable->errors()) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
}

