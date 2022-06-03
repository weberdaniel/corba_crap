// eg3_clt.cc - This is the source code of example 3 used in Chapter 2
//              "The Basics" of the omniORB user guide.
//
//              This is the client. It uses the COS naming service
//              to obtain the object reference.
//
// Usage: eg3_clt
//
//
//        On startup, the client lookup the object reference from the
//        COS naming service.
//
//        The name which the object is bound to is as follows:
//              root  [context]
//               |
//              text  [context] kind [my_context]
//               |
//              Echo  [object]  kind [Object]
//

#include <echo.hh>

#include <iostream>
using namespace std;

static void hello(Echo_ptr e)
{
  if (CORBA::is_nil(e)) {
    cerr << "hello: The object reference is nil!\n" << endl;
    return;
  }

  CORBA::String_var src = (const char*) "Hello!";

  CORBA::String_var dest = e->echoString(src);

  cerr << "I said, \"" << (char*)src << "\"." << endl
       << "The Echo object replied, \"" << (char*)dest <<"\"." << endl;
}

//////////////////////////////////////////////////////////////////////

int
main (int argc, char **argv) 
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    // We use a corbaname URI to resolve the name
    const char*       uri = "corbaname:rir:#test.my_context/Echo.Object";
    CORBA::Object_var obj = orb->string_to_object(uri);

    Echo_var echoref = Echo::_narrow(obj);

    for (CORBA::ULong count=0; count < 10; count++)
      hello(echoref);

    orb->destroy();
    return 0;
  }
  catch (CORBA::TRANSIENT&) {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "
         << "server." << endl;
  }
  catch (CORBA::NO_RESOURCES&) {
    cerr << "Caught NO_RESOURCES exception." << endl
         << "You must configure omniORB with the location of the naming service."
         << endl;
  }
  catch (CORBA::BAD_PARAM&) {
    cerr << "Caught BAD_PARAM exception." << endl
         << "The object is not registered in the naming service."
         << endl;
  }
  catch (CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name() << endl;
  }
  catch (CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  return 1;
}
