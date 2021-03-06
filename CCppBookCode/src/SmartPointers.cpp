// ==========================================================================
//
// Software written by Boguslaw Cyganek (C) to be used with the book:
// INTRODUCTION TO PROGRAMMING WITH C++ FOR ENGINEERS
// Published by Wiley, 2020
//
// The software is supplied as is and for educational purposes
// without any guarantees nor responsibility of its use in any application. 
//
// ==========================================================================




#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <functional>
#include <cassert>

#include "range.h"

using namespace std;
using CppBook::range;


///////////////////////////////////////////////////////////////////////////////////////////////


// An example of memory leak generation 
// DO NOT USE THIS CODE !!
#if 0
bool ProcessFile( const string fileName )
{
	const int kBufSize = 256;
	char * buf = new char[ kBufSize ];		// allocate a buffer on the heap

	ifstream	file( fileName );
	if( file.is_open() == false )
	{
		return false;	// cannot open a file, exiting ... 
	}						// oops! - buf is not destroyed - a memory leak is generated !

	file.read( buf, kBufSize );	// file open OK, read to the buffer
	// do something ...

	delete [] buf;		// delete the buffer

	return true;		// everything OK, exit
}
#endif 


// Use std::vector as an underlying data container (data allocated on the heap)
bool ProcessFile_1( const string fileName )
{
	const int kBufSize = 256;
	std::vector< char >	buf( kBufSize );		// allocate a buffer on the heap

	ifstream	file( fileName );
	if( file.is_open() == false )
	{
		return false;	// cannot open a file, exiting ... 
	}					// OK, buf is an automatic object, it wil be destroyed

	file.read( & buf[0], kBufSize );	// file open OK, read to the buffer
	// do something ...

	return true;		// everything OK, exit
} // OK, buf is an automatic object, it will be destroyed


// Use std::unique_ptr smart pointer object
bool ProcessFile_2( const string fileName )
{
	const int kBufSize = 256;
	auto buf = std::make_unique< char >( kBufSize );		// allocate a buffer on the heap
															// but hold it with a unique_ptr
	ifstream	file( fileName );
	if( file.is_open() == false )
	{
		return false;	// cannot open a file, exiting ... 
	}					// OK, buf is an automatic object, it wil be destroyed
						// and it will deallocate memory from the heap

	file.read( buf.get(), kBufSize );	// file open OK, read to the buffer (use get)
	// do something ...

	return true;		// everything OK, exit
} // OK, buf is an automatic object, it will be destroyed








// DO NOT USE IT IN OPERATIONAL CODE!
// This class is ONLY TO SIMULATE 
// behavior of the so called auto pointers.
// The main mechanism is RAII (Resource 
// Acquisition Is Initialization) - a pointer
// is get during construction of a_p, and the
// memory pointed by this pointer is AUTOMATICALLY
// released by the destructor of the a_p.
template < typename T >
class a_p
{
private:

	T *	fPtr;	// held ptr

public:

	a_p( T * p ) : fPtr( p ) {}

	// When a_p is destroyed, 
	// then also fPtr is destroyed.
	~a_p() { delete fPtr; }	

public:

	// a_p can be used as an ordinary pointer
	T & operator * ( void ) { return * fPtr; }

	// ...
};


void a_p_test( void )
{
	// Create advanced pointer as a local object
	// - make it hold a pointer to double
	a_p< double >	apd( new double( 0.0 ) );

	// Do something with apd like with an ordinary pointer to double
	* apd = 5.0;		
	* apd *= 3.0;
	cout << "apd=" << * apd << " sizeof(apd) = " << sizeof( apd ) << endl;

	// apd will be destroyed here since it is an automatic object
	// It will destroy held object as well
}	// <==




class AClass
{
	string fStr;
public:
	AClass( const string & s = "" ) : fStr(s) {}
	~AClass() { cout << "AClass destructor" << endl; }

	const string GetStr( void ) const { return fStr; }
	void SetStr( const string & s ) { fStr = s; }
};






void unique_ptr_tests( void )
{
	// Heap allocate a single double object, init to 0.0, and create a unique_ptr
	unique_ptr< double > real_val_0( new double( 0.0 ) );	// good

	// Heap allocate a single double object, init to 0.0, and create a unique_ptr
	// via make_unique. Better, can use "auto".
	auto real_val_1( make_unique< double >( 0.0 ) );		// better

	assert( real_val_1.get() != nullptr );	// check the allocated memory ptr
	* real_val_1 = 3.14;	// use like any other pointer

	// ------------------------------------------------------------------------
	// Heap allocate an array of 16 double � access via the unique_ptr
	const int kElems = 16;

	unique_ptr< double [] > 	real_array_0( new double[ kElems ] );	// OK
	// but elements of real_array_0 are NOT initialized !

	auto				real_array_1( make_unique< double [] >( kElems ) );	// better
	// Elements of real_array_1 will be value initialized with double(), i.e. to 0.0	
	
	// ------------------------------------------------------------------------
	if( real_array_0 )	// check if memory has been allocated (calls operator bool()) 
	{
		// init elems
		std::fill( & real_array_0[ 0 ], & real_array_0[ kElems ], -1.0 );
		real_array_0[ 0 ] = 2.71;	// use real_array_0 as a simple array
		// ...
	}

	real_array_0.reset();		// reset the held pointer and delete all elements



	// -----------------------------------------------
	// AClass object on the heap, p_d takes care
	unique_ptr< AClass > p_d_1( new AClass( "Good" ) );

	// A better way is to use make_unique helper
	auto p_d_2( make_unique< AClass >( "Better" ) );

	if( p_d_2 )
	{
		cout << p_d_2->GetStr() << endl;		// access through the pointer
		cout << ( * p_d_2 ).GetStr() << endl;	// access through the object
	}

	//unique_ptr< AClass > p_d_3 = p_d_2;	// won't work, cannot copy
	unique_ptr< AClass > p_d_3 = std::move( p_d_2 );	// OK, can move to other smart ptr

	// An array of AClass objects on the heap adn p_d_4 
	const int kAClassElems = 4;
	auto p_d_4( make_unique< AClass [] >( kAClassElems ) );	// call default constructor for each


}




template< typename P >
using unique_ptr_with_deleter = unique_ptr< P, function< void( P * ) > >;

void unique_ptr_with_custom_deleters( void )
{

	// -----------------------------------------------
	// Examples of custom deleters for unique_ptr

	// Define a custom deleter for AClass - a lambda function
	auto AClass_delete_fun = [] ( AClass * ac_ptr ) 
	{ 
		// Print to the log ...
		delete ac_ptr; 
	};

	// ...
	unique_ptr< AClass, decltype( AClass_delete_fun ) > p_d_8( new AClass(), AClass_delete_fun );

	// Define a custom deleter for FILE - a lambda function 
	auto file_close_fun = [] ( FILE * f) { fclose( f ); };


	// ...
	unique_ptr_with_deleter< FILE > file( fopen( "myFile.txt", "r" ), file_close_fun );

}




void things_to_avoid_with_unique_ptr( void )
{
	{																		
		// AClass object on the heap, p_d takes care
		auto p_d_0( new AClass( "No ..." ) );	// oops, an ordinary pointer

		delete p_d_0;			// get rid of it
	}

	{
		AClass * p_d_5 = new AClass;

		unique_ptr< AClass > p_d_6( p_d_5 );	// do not initialize via ord pointer

		// ...
		// somewhere else ...
		unique_ptr< AClass > p_d_7( p_d_5 );	// wrong, two unique_ptr to one object
	}



	{

		// Wrong!!! new on unique_ptr should be forbidden!
		unique_ptr< AClass > * pxx = new unique_ptr< AClass >( new AClass( "me" ) );
		using AClass_up = unique_ptr< AClass >;
		auto pzz = make_unique< AClass_up >( make_unique< AClass >( "" ) );

	}

}



// Simple factory example

// Pure virtual base class
class B
{
public:
	virtual ~B() {}

	virtual void operator() ( void ) = 0;
};

// Derived classes
class C : public B
{
public:
	virtual ~C() 
	{
		cout << "C is deleted" << endl;	
	}

	// It is also virtual but override is enough to express this (skip virtual)
	void operator() ( void ) override	
	{
		cout << "C is doing an action..." << endl;
	}
};

class D : public B
{
public:
	virtual ~D() 
	{
		cout << "D is deleted" << endl;	
	}

	void operator() ( void ) override
	{
		cout << "D is doing an action..." << endl;
	}
};

class E : public B
{
public:
	virtual ~E() 
	{
		cout << "E is deleted" << endl;	
	}

	void operator() ( void ) override
	{
		cout << "E is doing an action..." << endl;
	}
};




enum EClassId { kC, kD, kE };

auto Factory( EClassId id )
{
	switch( id )
	{
		case kC: return unique_ptr< B >( make_unique< C >() );
		case kD: return unique_ptr< B >( make_unique< D >() );
		case kE: return unique_ptr< B >( make_unique< E >() );

		default: assert( false );	// should not be here
	}

	return unique_ptr< B >();	// can be empty
}

void FactoryTest( void )
{
	vector< unique_ptr< B > > theObjects;// = { Factory( kC ), Factory( kD ), Factory( kE ) };

	theObjects.push_back( Factory( kC ) );		// copy or move semantics? move
	
	theObjects.emplace_back( Factory( kD ) );	// this for sure will use move semantics, OK

	theObjects.emplace_back( Factory( kE ) );

	theObjects[ theObjects.size() - 1 ] = Factory( kD );	// replace E with D

	for( auto & a : theObjects )
		( * a )();		// call actions via the virtual mechanism
}


/////////////////////////////////////////////////////////////////////

// More on passing to and from unique_ptr


class TMatrix
{
public:
	TMatrix( int cols, int rows ) {}
	// ... all the rest
};




// ---------
// PRODUCER
// Returns unique_ptr< TMatrix > 
auto	OrphanRandomMatrix( const int kCols, const int kRows /*, enum ERandType rand_type*/ )
{
	auto retMatrix( make_unique< TMatrix >( kCols, kRows ) );

	// ... do computations

	return retMatrix;		// return a heavy object using the move semantics
}


// ---------
// CONSUMERS


// If always processing an object, then pass an object by a const ref (read only)
double ComputeDeterminant( const TMatrix & matrix )
{
	double retDeterminant {};

	// ... do computations

	return retDeterminant;
}



// OK, unique_ptr is passed by a const reference, however
// a pointer to the matrix can be nullptr - we can use this feature
// if wish to have an option to pass an empty object
bool ComputeDeterminant( const unique_ptr< TMatrix > & matrix, double & outRetVal )
{
	if( ! matrix )
		return false;		// if an empty ptr, then no computations at all

	outRetVal = ComputeDeterminant( * matrix );		// get the object

	return true ;
}

// Pass by reference to the unique_ptr - we can take over a held object
void TakeOverAndProcessMatrix( unique_ptr< TMatrix > & matrix )
{
	// "it is a deleted function"
	unique_ptr< TMatrix > myMatrix( move( matrix ) );	// take over the object
														// changes the passed "matrix" to empty
	// ... do computations

	// when we exit there will be no matrix object at all
}

// Such a version is also possible - however, we can have
// matrix == nullptr, then it is also possible that matrix.get() == nullptr
void TakeOverAndProcessMatrix( unique_ptr< TMatrix > * matrix )
{
	if( matrix == nullptr )
		return;

	// "it is a deleted function"
	unique_ptr< TMatrix > myMatrix( move( * matrix ) );	// take over the object
														// changes the passed "matrix" to empty
	// ... do computations

	// when we exit there will be no matrix object at all
}


void AcceptAndProcessMatrix( unique_ptr< TMatrix > matrix )
{
	// if here, then TMatrix object is governed by the 
	// local matrix unique_ptr - it is owned by this function

	assert( matrix );


	// ... do computations

	// when we exit there will be no matrix object at all
}


void unique_ptr_OrphanAcceptTest( void )
{
	auto	matrix_1( OrphanRandomMatrix( 20, 20 ) );
	// matrix_1 is of type unique_ptr< TMatrix >

	assert( matrix_1 );		// make sure the object was created OK (enough memory,  etc.)

	cout << "Det = " << ComputeDeterminant( * matrix_1 ) << endl;


	double determinant {};
	bool detCompStatus = ComputeDeterminant( matrix_1, determinant );

	assert( detCompStatus );


	TakeOverAndProcessMatrix( matrix_1 );		// this will take over the TMatrix object from the passed unique_ptr
	//TakeOverAndProcessMatrix( & matrix_1 );		// this will take over the TMatrix object from the passed unique_ptr

	assert( ! matrix_1 );		// no object, only an empty unique_ptr


	matrix_1 = make_unique< TMatrix >( 20, 20 );	// create other fresh object (move semantics)

	assert( matrix_1 );

	//AcceptAndProcessMatrix( matrix_1 );	// generates an error - "attempting to reference a deleted function"
	AcceptAndProcessMatrix( move( matrix_1 ) );	// we need to force the move semantics

	assert( ! matrix_1 );		// no object, only an empty unique_ptr

	AcceptAndProcessMatrix( OrphanRandomMatrix( 20, 20 ) );		// however, we can make and pass a temporary object

}



/////////////////////////////////////////////////////////////////////



void shared_ptr_test( void )
{

	auto sp_0 = make_shared< AClass >( "Hold by shared" );	// a control block will be created

	assert( sp_0 );			// sp_0 cast to bool to check if it is not empty
	assert( sp_0.get() != nullptr );			// the same

	auto sp_1 = sp_0;		// copy for shared is OK - now both point at the same
							// object, one control block, and the reference counter is 2


	cout << sp_0->GetStr() << " = " << (*sp_1).GetStr() << endl;	// accessing the same object
	cout << "sp_0 count = " << sp_0.use_count() << ", sp_1 count = " << sp_1.use_count() << endl;

	sp_0.reset();
	cout << "after reset sp_0 count = " << sp_0.use_count() << ", sp_1 count = " << sp_1.use_count() << endl;



	//---------------------------------------------------
	// shared_ptr can be made out of the unique_ptr

	auto up_0 = make_unique< AClass >( "Created by unique" );

	shared_ptr< AClass > sp_2 = move( up_0 ); 

	auto sp_3 = sp_2;

	cout << sp_2->GetStr() << " = " << (*sp_3).GetStr() << endl;	// accessing the same object
	cout << "sp_2 count = " << sp_2.use_count() << ", sp_3 count = " << sp_3.use_count() << endl;


	//---------------------------------------------------
	// shared_ptr to an array

	const int kElems = 8;
	shared_ptr< AClass [] >	sp_4( new AClass[ kElems ] );
	
	cout << "sp_4 count = " << sp_4.use_count() << endl;

	for( int i = 0; i < kElems; ++ i )
		cout << sp_4[ i ].GetStr() << ", ";

	cout << endl;

	shared_ptr< AClass [] >	sp_5( sp_4 );
	cout << "sp_4 count = " << sp_4.use_count() << ", sp_5 count = " << sp_5.use_count() <<  endl;

}


using shared_ptr_2_buffer = shared_ptr< AClass [] >;

void acceptor_0( shared_ptr_2_buffer sh, const int num_of_elems )
{
	cout << "sh.use_count() = " << sh.use_count() << endl;

	// Work with data ...
	if( num_of_elems > 2 )
		cout << sh[ 2 ].GetStr() << endl;

}

void acceptor_1( shared_ptr_2_buffer sh, const int num_of_elems )
{
	cout << "sh.use_count() = " << sh.use_count() << endl;

	// Work with data ...
	if( num_of_elems > 3 )
		cout << sh[ 3 ].GetStr() << endl;


	acceptor_0( sh, num_of_elems );
}



void donator( void )
{
	const int kBufLen = 4;
	shared_ptr_2_buffer	sh( new AClass[ kBufLen ] );

	for( auto i : range( kBufLen ) )
		sh[ i ].SetStr( to_string( i ) );	// init, from numbers to strings
	
	cout << "sh.use_count() = " << sh.use_count() << endl;

	// Share the buffer with other functions

	acceptor_1( sh, kBufLen );

	// Work with data ...


	cout << "sh.use_count() = " << sh.use_count() << endl;

}


///////////////////////////////////////////////////////////////////////////
// weak_ptr test

class N
{
	string fStr;
public:	
	const string & GetStr( void ) const { return fStr; }
	void SetStr( const string & s ) { fStr = s; }

private:	
	shared_ptr< N > fNext;	// forward reference
	weak_ptr< N >	fPrev;	// back reference

public:	
	void SetNext( shared_ptr< N > s ) { fNext = s; }
	//void SetPrev( weak_ptr< N >	p ) { fPrev = p; }
	shared_ptr< N > GetNext( void ) const { return fNext; }
public:		
	
	N( const string & s = "", const shared_ptr< N > & prev = nullptr ) : fStr( s ), fPrev( prev ) {}
	~N() { cout << "Killing node " << fStr << endl; }

public:	
	// Add 3 texts ... - [i-1] - [i] - [i+1] - ...
	string operator() ( void )
	{
		string outStr;

		// fPrev is a weak_ptr
		if( fPrev.expired() == false ) outStr += fPrev.lock()->GetStr();
		outStr += fStr;
		// fNext is a shared ptr
		if( fNext ) outStr += fNext->GetStr();

		return outStr;
	}
};




void double_linked_list_test( void )
{
	using SP_N = shared_ptr< N >;	// a useful type alias
	

	SP_N root, pr;		// two empty shared ptrs 

	// ---------------
	// Create the list
	for( const auto & s : { "A", "B", "C", "D", "E", "F" } )
	{
		if( ! pr /*== false*/ )
		{
			root = pr = make_shared< N >( s );	// the first node 
		}
		else
		{
			// Make a new node, and connect to the previous node in the list
			pr->SetNext( make_shared< N >( s, pr ) );
			pr = pr->GetNext();		// advance pr on to the end of the list
		}
	}

	// ---------------------------------------------------------------
	// OK, the list is ready, so traverse the list and call operator()
	// To check if pointing at valid object implicitly use operator bool ()
	//                  v
	for( SP_N p = root; p; p = p->GetNext() )
		cout << ( * p )() << endl;
}

// Pass shared_ptr by value to create another link to an object
void ProcessObject_0( shared_ptr< AClass > sp )
{
	cout << "sp.use_count = " << sp.use_count() << endl;

	// do something with sp ...

	sp.reset();	// sp is local to this function

	cout << "sp.use_count = " << sp.use_count() << endl;
}

// Pass shared_ptr by a reference to change that shared_ptr
void ProcessObject_1( shared_ptr< AClass > & sp )
{
	cout << "sp.use_count = " << sp.use_count() << endl;

	// do something with sp ...

	sp.reset();	// sp is local to this function

	cout << "sp.use_count = " << sp.use_count() << endl;
}

void weak_ptr_test( void )
{
	auto sp_0 = make_shared< AClass >( "Goose" );
	cout << "sp_0 created" << endl;

	cout << "\tsp_0.use_count = " << sp_0.use_count() << endl;

	weak_ptr< AClass > wp_0( sp_0 ); // create a weak_ptr assoc with sp_0
	cout << "wp_0 created" << endl;

	auto sp_1 = sp_0;
	cout << "sp_1 created" << endl;

	cout << "\tsp_0.use_count = " << sp_0.use_count() << endl; // there is one 
	cout << "\tsp_1.use_count = " << sp_1.use_count() << endl; // control block

	// Check if the object is still alive
	if( wp_0.expired() != true )
	{
		// We can access the object via lock()
		cout << wp_0.lock()->GetStr() << endl;
	}

	assert( sp_0 );	// assert that the main object is OK

	cout << sp_0->GetStr() << endl;	// main object still OK

	cout << "sp_0.reset()" << endl;
	sp_0.reset();	// detach sp_0

	cout << "\tsp_0.use_count = " << sp_0.use_count() << endl; // there is one 
	cout << "\tsp_1.use_count = " << sp_1.use_count() << endl; // control block
	cout << ( wp_0.expired() ? "\twp_0 expired" : "\twp_0 not expired" ) << endl;

	cout << "wp_0.reset()" << endl;
	wp_0.reset();	// detach (only) wp_0 from the control block 
						// - does not affect the held object

	assert( sp_1 );	// assert that the main object is OK but through sp_1
	cout << sp_1->GetStr() << endl;	// use the main object via sp_1


	cout << "\tsp_0.use_count = " << sp_0.use_count() << endl; 
	cout << "\tsp_1.use_count = " << sp_1.use_count() << endl; 
	cout << ( wp_0.expired() ? "\twp_0 expired" : "\twp_0 not expired" ) << endl;

	cout << "sp_1.reset()" << endl;	
	sp_1.reset();	// detach sp_1


	cout << "\tsp_0.use_count = " << sp_0.use_count() << endl; 
	cout << "\tsp_1.use_count = " << sp_1.use_count() << endl; 
	cout << ( wp_0.expired() ? "\twp_0 expired" : "\twp_0 not expired" ) << endl;


	//ProcessObject_0( sp_0 );

	//ProcessObject_1( sp_0 );

}




// -----------------------------------------------------------------



// Cyclic dependence problem

struct Bar;

struct Foo
{
	std::shared_ptr< Bar > bar;
	~Foo() { std::cout << "~Foo()\n";}
};

struct Bar
{
	std::shared_ptr< Foo > foo; // Here we have a problem ... 
	//std::weak_ptr< Foo > foo; // A solution
	~Bar() { std::cout << "~Bar()\n";}
};

void CyclicPointersProblem_Test( void )
{
	auto foo = make_shared< Foo >();
	foo->bar = make_shared< Bar >();

	foo->bar->foo = foo;	// oops, a circle ...
}













