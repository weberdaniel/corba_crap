class Echo_i : public POA_Echo
{
public:
  inline Echo_i() {}
  virtual ~Echo_i() {}
  virtual char* echoString(const char* mesg);
};

char* Echo_i::echoString(const char* mesg)
{
  return CORBA::string_dup(mesg);
}
