
//////////////////////////////////////////////////////////////////////////
////////////////////////////     ROOT API     ////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <sstream>

#include "TDbiFieldType.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiString.hxx"
#include "TDbiStatement.hxx"
#include "TDbiTableMetaData.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
using std::istringstream;
using std::ostringstream;
#include "UtilString.hxx"
#include "TVldTimeStamp.hxx"

ClassImp(ND::TDbiInRowStream)


//   Definition of static data members
//   *********************************


//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.


//.....................................................................
///\verbatim
///
///  Purpose:  Default constructor
///
///  Arguments:
///     stmtDb     in  ND::TDbiStatement to be used for query.  May be zero.
///     sql        in  The query to be applied to the statement.
///     metaData   in  Meta data for query.
///     tableProxy in  Source ND::TDbiTableProxy.
///     dbNo       in  Cascade no. of source.
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Create ResultSet for query.
///\endverbatim

ND::TDbiInRowStream::TDbiInRowStream(ND::TDbiStatement* stmtDb,
                           const ND::TDbiString& sql,
                           const ND::TDbiTableMetaData* metaData,
                           const ND::TDbiTableProxy* tableProxy,
                           UInt_t dbNo,
                           const string& fillOpts) :
ND::TDbiRowStream(metaData),
fCurRow(0),
fDbNo(dbNo),
fStatement(stmtDb),
fTSQLStatement(0),
fExhausted(true),
fTableProxy(tableProxy),
fFillOpts(fillOpts)
{


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiInRowStream" << "  ");

  if ( stmtDb ) {
    fTSQLStatement = stmtDb->ExecuteQuery(sql.c_str());
    if ( fTSQLStatement && fTSQLStatement->NextResultRow() ) fExhausted = false;
    stmtDb->PrintExceptions(ND::TDbiLog::DebugLevel );
  }

}


//.....................................................................
///\verbatim
///
///  Purpose: Destructor
///
///  Arguments:
///    None.
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Destroy ResultSet and owned ND::TDbiStatement if any.
///\endverbatim

ND::TDbiInRowStream::~TDbiInRowStream() {

//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Destroying ND::TDbiInRowStream" << "  ");
  delete fTSQLStatement;
  fTSQLStatement = 0;
  delete fStatement;
  fStatement = 0;

}

//.....................................................................


#define IN(t) istringstream in(AsString(t)); in

// On first row use AsString to force type checking.
// On subsequent rows use binary interface for speed.
// Caution: Column numbering in TSQLStatement starts at 0.
#define IN2(t,m)                            \
  int col = CurColNum()-1;                  \
  if ( CurRowNum() == 0 ) {                 \
    istringstream in(AsString(t));          \
    in >> dest;                             \
  }                                         \
  else {                                    \
    dest = fTSQLStatement->m(col);          \
    IncrementCurCol();                      \
  }                                         \

// Handling reading of unsigned application data stored as signed database data.
// Both GetInt(int) and GetString(int) return the signed data correctly.
// So first read into signed equivalent, then copy and finally
// trim off leading extended sign bits beyond the capacity of
// the database column.
// For BIGINT (size 8) make an exception.  It's used only as
// an alternative to unsigned int and getUInt(int) (but not GetInt(int))
// returns it correctly so can load directly into destination
// Caution: Column numbering in TSQLStatement starts at 0.
#define IN3(t)                                                      \
int col = this->CurColNum()-1;                                      \
const ND::TDbiFieldType& fType = this->ColFieldType(col+1);              \
if ( fType.GetSize() == 8 ) {                                       \
  dest=fTSQLStatement->GetUInt(col);				    \
}                                                                   \
else {                                                              \
  t dest_signed;                                                    \
  *this >> dest_signed;                                             \
  dest = dest_signed;                                               \
  if ( fType.GetSize() == 1 ) dest &= 0xff;                         \
  if ( fType.GetSize() == 2 ) dest &= 0xffff;                       \
  if ( fType.GetSize() == 4 ) dest &= 0xffffffff;                   \
}\

ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(Bool_t& dest) {
                                 IN(TDbi::kBool) >> dest;  return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(Char_t& dest) {
                                 IN(TDbi::kChar) >> dest; return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(Short_t& dest) {
                                 IN2(TDbi::kInt,GetInt);    return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(UShort_t& dest) {
                                 IN3(Short_t); return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(Int_t& dest) {
                                 IN2(TDbi::kInt,GetInt);      return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(UInt_t& dest) {
                                 IN3(Int_t);  return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(Long_t& dest) {
                                 IN2(TDbi::kLong, GetLong);   return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(ULong_t& dest) {
                                 IN3(Long_t);  return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(Float_t& dest) {
                                 IN2(TDbi::kFloat,GetDouble);  return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(Double_t& dest) {
                                 IN2(TDbi::kDouble,GetDouble);return *this;}

// Also use AsString() for string and ND::TVldTimeStamp; conversion to string
// is needed in any case.
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(string& dest) {
                          dest = AsString(TDbi::kString);  return *this;}
ND::TDbiInRowStream& ND::TDbiInRowStream::operator>>(ND::TVldTimeStamp& dest){
           dest=TDbi::MakeTimeStamp(AsString(TDbi::kDate)); return *this;}

//.....................................................................
///\verbatim
///  Purpose: Return current column value as a modifiable string and
///           move on.
///
///  Arguments:
///    type         in    Required data type (as defined in TDbi.hxx).
///
///  Return:   Current column value as a string (null if missing)
///            Note: Caller must dispose of value before calling
///                  this member function again as the value is
///                  assembled into fValString.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
/// o Return the datum at current (row,column) as a string and
///   increment column number.
///
/// o Check for compatibility between required data type and table
///   data type, report problems and return default if incompatible.
///\endverbatim
string& ND::TDbiInRowStream::AsString(TDbi::DataTypes type) {
//

//  Program Notes:-
//  =============

//  None.

  ND::TDbiFieldType  reqdt(type);

//  Place table value string in value string buffer.

  Bool_t fail = ! LoadCurValue();
// Internally columns number from zero.
  UInt_t col = CurColNum();
  IncrementCurCol();

  if ( fail ) {
    string udef = reqdt.UndefinedValue();
       DbiSevere(  "... value \"" << udef
       << "\" will be substitued." <<  "  ");
    fValString = udef;
    return fValString;
  }

//  Check for compatibility with required data type.

  const ND::TDbiFieldType& actdt = MetaData()->ColFieldType(col);

  if ( reqdt.IsCompatible(actdt) ) {
    Bool_t smaller = reqdt.IsSmaller(actdt);
//  Allow one character String to be stored in Char
    if ( reqdt.GetConcept() == TDbi::kChar && fValString.size() == 1
       ) smaller = kFALSE;
    if ( smaller  ) {
          DbiWarn(  "In table " << TableNameTc()
          << " row " << fCurRow
          << " column "<< col
          << " (" << MetaData()->ColName(col) << ")"
	  << " value \"" << fValString
	  << "\" of type " << actdt.AsString()
	  << " may be truncated before storing in " << reqdt.AsString()
          <<  "  ");
    }
  }
  else {
    string udef = reqdt.UndefinedValue();
       DbiSevere(  "In table " << TableNameTc()
         << " row " << fCurRow
        << " column "<< col
        << " (" << MetaData()->ColName(col) << ")"
	<< " value \"" << fValString
        << "\" of type " << actdt.AsString()
	<< " is incompatible with user type " << reqdt.AsString()
        << ", value \"" << udef
        << "\" will be substituted." <<  "  ");
    fValString = udef;
  }

  return fValString;
}


//.....................................................................
///\verbatim
///
///  Purpose:  Test if current column exists.
///
///  Arguments: None.
///
///  Return:    kTRUE if column exists.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Test if current column exists.
///\endverbatim
Bool_t ND::TDbiInRowStream::CurColExists() const {

//  Program Notes:-
//  =============

//  None.

  Int_t col = CurColNum();

  if ( IsExhausted() ) {
       DbiSevere(  "In table " << TableNameTc()
      << " attempting to access row " << fCurRow
      << " column " << col
      << " but only " << fCurRow << " rows in table."  << "  ");
    return kFALSE;
  }

  int numCols = NumCols();
  if ( col > numCols ) {
       DbiSevere(  "In table " << TableNameTc()
      << " row " << fCurRow
      << " attempting to access column "<< col
      << " but only " << NumCols() << " in table ."  << "  ");
    return kFALSE;
  }

  return kTRUE;

}
//.....................................................................
///\verbatim
///
///  Purpose:  Return current column as a string.
///
///  Arguments: None.
///
///  Return:   Current column as a string (or null string if non-existant).
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Return current column as a string.
///\endverbatim
string ND::TDbiInRowStream::CurColString() const {

//  Program Notes:-
//  =============

//  None.

  if ( ! CurColExists() ) return "";

  TString valStr = this->GetStringFromTSQL(CurColNum());
  return valStr.Data();

}

//.....................................................................
///\verbatim
///
///  Purpose: Fetch next row of result set..
///
///  Arguments: None.
///
///  Return:   kTRUE if row exists, kFALSE otherwise.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Load next row with string lengths.
///\endverbatim
Bool_t ND::TDbiInRowStream::FetchRow() {


//  Program Notes:-
//  =============

//  None.

  ClearCurCol();
  if ( IsExhausted() ) return kFALSE;
  ++fCurRow;
  if ( ! fTSQLStatement->NextResultRow() ) fExhausted = true;
  return ! fExhausted;

}
//.....................................................................
///\verbatim
///
///  Purpose: Get string from underlying TSQL interface
///
///  N.B.  No check that col is valid - caller beware.
///
///  Specification:-
///  =============
///
///  o Get string from underlying TSQL interface.
///
///\endverbatim
TString ND::TDbiInRowStream::GetStringFromTSQL(Int_t col) const {

// Caution: Column numbering in TSQLStatement starts at 0.
  TString valStr = fTSQLStatement->GetString(col-1);
  return valStr;
}

//.....................................................................
///\verbatim
///
///  Purpose:  Load current value into buffer fValString
///
///  Arguments: None.
///
///  Return:    kTRUE if current column in range, otherwise kFALSE.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Load current value into buffer fValString stripping off any
///    enclosing quotes.
///\endverbatim
Bool_t ND::TDbiInRowStream::LoadCurValue() const{


  fValString.clear();

  if ( ! CurColExists() ) return kFALSE;

  Int_t col = CurColNum();
  TString valStr = this->GetStringFromTSQL(col);

  // For floating point, use binary interface to preserve precision
  // e.g.-1.234567890123457e-100 as string is -0.000000
  if ( CurColFieldType().GetConcept() == TDbi::kFloat ) {
      std::ostringstream out;
      out << std::setprecision(8);
      if ( CurColFieldType().GetType() == TDbi::kDouble )  out << std::setprecision(16);
//  Caution: Column numbering in TSQLStatement starts at 0.
    out << fTSQLStatement->GetDouble(col-1);
    valStr = out.str().c_str();
  }
  int len = valStr.Length();



  const char* pVal = valStr.Data();
  // Remove leading and trailing quotes if dealing with a string.
  if (    len >= 2
       && ( *pVal == *(pVal+len-1) )
       && ( *pVal == '\'' || *pVal == '"' ) ) {
    ++pVal;
    len -= 2;
  }
  fValString.assign(pVal,len);

  return kTRUE;

}
//.....................................................................
///\verbatim
///
///  Purpose:  Append row as a Comma Separated Values string.
///
///  Arguments:
///    row          in    String to append to.
///
///\endverbatim
void ND::TDbiInRowStream::RowAsCsv(string& row) const {

  const ND::TDbiTableMetaData* md = this->MetaData();

  Int_t maxCol = this->NumCols();
  for (Int_t col = 1; col <= maxCol; ++col) {
    // Deal with NULL values.  Caution: Column numbering in TSQLStatement starts at 0.
    if ( fTSQLStatement->IsNull(col-1) ) {
      row += "NULL";
      if ( col < maxCol ) row += ',';
      continue;
    }
    Bool_t mustDelimit  = md->ColMustDelimit(col);
    UInt_t concept      = md->ColFieldConcept(col);
    if ( mustDelimit ) row += '\'';
    TString str = this->GetStringFromTSQL(col);
    const char* value = str.Data();

    // Make strings printable.
    if ( concept == TDbi::kString ) ND::UtilString::MakePrintable(value,row);

    // For floating point, use binary interface to preserve precision
    // e.g.-1.234567890123457e-100 as string is -0.000000
    else if ( concept == TDbi::kFloat ) {
      ostringstream out;
      out << std::setprecision(8);
      if ( md->ColFieldType(col).GetType() == TDbi::kDouble ) out << std::setprecision(16);
      out << fTSQLStatement->GetDouble(col-1);
      row += out.str();
    }

    // Everything else (!) is O.K.
    else                        row += value;

    if ( mustDelimit ) row += '\'';
    if ( col < maxCol ) row += ',';
  }
}



