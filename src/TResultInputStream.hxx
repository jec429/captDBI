#ifndef TResultInputStream_hxx_seen
#define TResultInputStream_hxx_seen

#include "TDbiInRowStream.hxx"
namespace ND {
class TVldTimeStamp;
};

namespace ND {
    class TResultInputStream;
};

/// This is the class used to fill table rows from query results.

class ND::TResultInputStream {

public:

    TResultInputStream(TDbiInRowStream& rs) : fResultSet(rs) {}
    virtual ~TResultInputStream() {};

    template <class T> TResultInputStream& operator>>(T& dest) {
	fResultSet >> dest; 
	return *this;
    }

private:

    TDbiInRowStream& fResultSet;
    ClassDef(TResultInputStream,1)

};

#endif
