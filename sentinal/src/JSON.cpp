/* Copyright (c) 2007-2008 John W Wilkinson
  2  
  3     This source code can be used for any purpose as long as
  4     this comment is retained. */
  5  
  6  // json spirit version 2.03
  7  
  8  #include "json_spirit.h"
  9  #include <cassert>
 10  #include <algorithm>
 11  #include <fstream>
 12  #include <boost\bind.hpp>
 13  
 14  using namespace std;
 15  using namespace boost;
 16  using namespace json_spirit;
 17  
 18  void add_pair( Object& obj, const string& name, const Value& value )
 19  {
 20      obj.push_back( Pair( name, value ) );
 21  }
 22  
 23  // adds a new child object to an existing object,
 24  // returns the new child object
 25  //
 26  Object& add_child_obj( Object& parent, const string& name )
 27  {
 28      parent.push_back( Pair( name, Object() ) );
 29  
 30      return parent.back().value_.get_obj();
 31  }
 32  
 33  // adds a new child object to an existing array,
 34  // returns the new child object
 35  //
 36  Object& add_child_obj( Array& parent )
 37  {
 38      parent.push_back( Object() );
 39  
 40      return parent.back().get_obj();
 41  }
 42  
 43  Array& add_array( Object& obj, const string& name )
 44  {
 45      obj.push_back( Pair( name, Array() ) );
 46  
 47      return obj.back().value_.get_array();
 48  }
 49  
 50  bool same_name( const Pair& pair, const string& name )
 51  {
 52      return pair.name_ == name;
 53  }
 54  
 55  const Value& find_value( const Object& obj, const string& name )
 56  {
 57      Object::const_iterator i = find_if( obj.begin(), obj.end(), bind( same_name, _1, ref( name ) ) );
 58  
 59      if( i == obj.end() ) return Value::null;
 60  
 61      return i->value_;
 62  }
 63  
 64  const Array& find_array( const Object& obj, const string& name )
 65  {
 66      return find_value( obj, name ).get_array(); 
 67  }
 68  
 69  int find_int( const Object& obj, const string& name )
 70  {
 71      return find_value( obj, name ).get_int();
 72  }
 73  
 74  string find_str( const Object& obj, const string& name )
 75  {
 76      return find_value( obj, name ).get_str();
 77  }
 78  
 79  struct Address
 80  {
 81      bool operator==( const Address& lhs ) const
 82      {
 83          return ( house_number_ == lhs.house_number_ ) &&
 84                 ( road_         == lhs.road_ ) &&
 85                 ( town_         == lhs.town_ ) &&
 86                 ( county_       == lhs.county_ ) &&
 87                 ( country_      == lhs.country_ );
 88      }
 89  
 90      int house_number_;
 91      string road_;
 92      string town_;
 93      string county_;
 94      string country_;
 95  };
 96  
 97  void write_address( Array& a, const Address& addr )
 98  {
 99      Object& addr_obj( add_child_obj( a ) );
100  
101      add_pair( addr_obj, "house_number", addr.house_number_ );
102      add_pair( addr_obj, "road",         addr.road_ );
103      add_pair( addr_obj, "town",         addr.town_ );
104      add_pair( addr_obj, "county",       addr.county_ );
105      add_pair( addr_obj, "country",      addr.country_ );
106  
107      // an alternative method to do the above would be as below,
108      // but the push_back causes a copy of the object to made
109      // which could be expensive for complex objects
110      //
111      //Object addr_obj;
112  
113      //add_pair( addr_obj, "house_number", addr.house_number_ );
114      //add_pair( addr_obj, "road",         addr.road_ );
115      //add_pair( addr_obj, "town",         addr.town_ );
116      //add_pair( addr_obj, "county",       addr.county_ );
117      //add_pair( addr_obj, "country",      addr.country_ );
118  
119      //a.push_back( addr_obj );
120  }
121  
122  Address read_address( const Object& obj )
123  {
124      Address addr;
125  
126      addr.house_number_ = find_int( obj, "house_number" );
127      addr.road_         = find_str( obj, "road" );        
128      addr.town_         = find_str( obj, "town" );        
129      addr.county_       = find_str( obj, "county" );       
130      addr.country_      = find_str( obj, "country" );      
131  
132      return addr;
133  }
134  
135  void write_addrs( const char* file_name, const Address addrs[] )
136  {
137      Object root_obj;
138  
139      Array& addr_array( add_array( root_obj, "addresses" ) );
140  
141      for( int i = 0; i < 5; ++i )
142      {
143          write_address( addr_array, addrs[i] );
144      }
145  
146      ofstream os( file_name );
147  
148      write_formatted( root_obj, os );
149  
150      os.close();
151  }
152  
153  vector< Address > read_addrs( const char* file_name )
154  {
155      ifstream is( file_name );
156  
157      Value value;
158  
159      read( is, value );
160  
161      Object root_obj( value.get_obj() );  // a 'value' read from a stream could be an JSON array or object
162  
163      const Array& addr_array( find_array( root_obj, "addresses" ) );
164  
165      vector< Address > addrs;
166  
167      for( unsigned int i = 0; i < addr_array.size(); ++i )
168      {
169          addrs.push_back( read_address( addr_array[i].get_obj() ) );
170      }
171  
172      return addrs;
173  }
174  
175  int main()
176  {
177      const Address addrs[5] = { { 42, "East Street",  "Newtown",     "Essex",        "England" },
178                                 { 1,  "West Street",  "Hull",        "Yorkshire",    "England" },
179                                 { 12, "South Road",   "Aberystwyth", "Dyfed",        "Wales"   },
180                                 { 45, "North Road",   "Paignton",    "Devon",        "England" },
181                                 { 78, "Upper Street", "Ware",        "Hertforshire", "England" } };
182  
183      const char* file_name( "demo.txt" );
184  
185      write_addrs( file_name, addrs );
186  
187      vector< Address > new_addrs( read_addrs( file_name ) );
188  
189      assert( new_addrs.size() == 5 );
190  
191      for( int i = 0; i < 5; ++i )
192      {
193          assert( new_addrs[i] == addrs[i] );
194      }
195  
196      return 0;
197  }
