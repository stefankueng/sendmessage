#pragma once
#include "StringUtils.h"
#include <string>
#include <vector>
#include <deque>

struct _tagXMLAttr;
typedef _tagXMLAttr XAttr, *LPXAttr;
typedef std::vector<LPXAttr> XAttrs;

struct _tagXMLNode;
typedef _tagXMLNode XNode, *LPXNode;
typedef std::vector<LPXNode> XNodes, *LPXNodes;

struct _tagXMLDocument;
typedef struct _tagXMLDocument XDoc, *LPXDoc;

// Entity Encode/Decode Support
typedef struct _tagXmlEntity
{
    TCHAR entity;                   // entity ( & " ' < > )
    TCHAR ref[10];                  // entity reference ( &amp; &quot; etc )
    int ref_len;                    // entity reference length
}XENTITY,*LPXENTITY;

typedef struct _tagXMLEntitys : public std::vector<XENTITY>
{
    LPXENTITY GetEntity( int entity );
    LPXENTITY GetEntity( LPTSTR entity );
    size_t GetEntityCount( LPCTSTR str );
    size_t Ref2Entity( LPCTSTR estr, LPTSTR str, size_t strlen );
    size_t Entity2Ref( LPCTSTR str, LPTSTR estr, size_t estrlen );
    std::wstring Ref2Entity( LPCTSTR estr );
    std::wstring Entity2Ref( LPCTSTR str );

    _tagXMLEntitys(){};
    _tagXMLEntitys( LPXENTITY entities, int count );
}XENTITYS,*LPXENTITYS;
extern XENTITYS entityDefault;
std::wstring XRef2Entity( LPCTSTR estr );
std::wstring XEntity2Ref( LPCTSTR str );

typedef enum
{
    PIE_PARSE_WELFORMED = 0,
    PIE_ALONE_NOT_CLOSED,
    PIE_NOT_CLOSED,
    PIE_NOT_NESTED,
    PIE_ATTR_NO_VALUE
}PCODE;

// Parse info.
typedef struct _tagParseInfo
{
    bool        trim_value;         // [set] do trim when parse?
    bool        entity_value;       // [set] do convert from reference to entity? ( &lt; -> < )
    LPXENTITYS  entities;           // [set] entity table for entity decode
    TCHAR       escape_value;       // [set] escape value (default '\\')
    bool        force_parse;        // [set] force parse even if xml is not welformed

    LPTSTR      xml;                // [get] xml source
    bool        error_occur;        // [get] is occurance of error?
    LPTSTR      error_pointer;      // [get] error position of xml source
    PCODE       error_code;         // [get] error code
    std::wstring    error_string;   // [get] error string

    LPXDoc      doc;
    _tagParseInfo() { trim_value = false; entity_value = true; force_parse = false; entities = &entityDefault; xml = NULL; doc = NULL; error_occur = false; error_pointer = NULL; error_code = PIE_PARSE_WELFORMED; escape_value = '\\'; }
}PARSEINFO,*LPPARSEINFO;
extern PARSEINFO piDefault;

// display optional environment
typedef struct _tagDispOption
{
    bool newline;           // newline when new tag
    bool reference_value;   // do convert from entity to reference ( < -> &lt; )
    TCHAR value_quotation_mark; // val="" (default value quotation mark "
    LPXENTITYS  entities;   // entity table for entity encode

    int tab_base;           // internal usage
    _tagDispOption() { newline = true; reference_value = true; entities = &entityDefault; tab_base = 0; value_quotation_mark = '"'; }
}DISP_OPT, *LPDISP_OPT;
extern DISP_OPT optDefault;

// XAttr : Attribute Implementation
typedef struct _tagXMLAttr
{
    std::wstring name;
    std::wstring    value;

    _tagXMLNode*    parent;

    std::wstring GetXML( LPDISP_OPT opt = &optDefault );
}XAttr, *LPXAttr;

typedef enum
{
    XNODE_ELEMENT,              // general node '<element>...</element>' or <element/>
    XNODE_PI,                   // <?xml version="1.0" ?>
    XNODE_COMMENT,              // <!-- comment -->
    XNODE_CDATA,                // <![CDATA[ cdata ]]>
    XNODE_DOC,                  // internal virtual root
}NODE_TYPE;

// XMLNode structure
typedef struct _tagXMLNode
{
    // name and value
    std::wstring    name;
    std::wstring    value;

    // internal variables
    LPXNode parent;     // parent node
    XNodes  childs;     // child node
    XAttrs  attrs;      // attributes
    NODE_TYPE type;     // node type
    LPXDoc  doc;        // document

    // Load/Save XML
    LPTSTR  Load( LPCTSTR pszXml, LPPARSEINFO pi = &piDefault );
    std::wstring GetXML( LPDISP_OPT opt = &optDefault );
    std::wstring GetText( LPDISP_OPT opt = &optDefault );

    // internal load functions
    LPTSTR  LoadAttributes( LPCTSTR pszAttrs, LPPARSEINFO pi = &piDefault );
    LPTSTR  LoadAttributes( LPCTSTR pszAttrs, LPCTSTR pszEnd, LPPARSEINFO pi = &piDefault );
    LPTSTR  LoadProcessingInstrunction( LPCTSTR pszXml, LPPARSEINFO pi = &piDefault );
    LPTSTR  LoadComment( LPCTSTR pszXml, LPPARSEINFO pi = &piDefault );
    LPTSTR  LoadCDATA( LPCTSTR pszXml, LPPARSEINFO pi = &piDefault );

    // in own attribute list
    LPXAttr GetAttr( LPCTSTR attrname );
    LPCTSTR GetAttrValue( LPCTSTR attrname );
    XAttrs  GetAttrs( LPCTSTR name );

    // in one level child nodes
    LPXNode GetChild( LPCTSTR name );
    LPCTSTR GetChildValue( LPCTSTR name );
    std::wstring    GetChildText( LPCTSTR name, LPDISP_OPT opt = &optDefault );
    XNodes  GetChilds( LPCTSTR name );
    XNodes  GetChilds();

    LPXAttr GetChildAttr( LPCTSTR name, LPCTSTR attrname );
    LPCTSTR GetChildAttrValue( LPCTSTR name, LPCTSTR attrname );

    // search node
    LPXNode Find( LPCTSTR name );

    // modify DOM
    size_t  GetChildCount();
    LPXNode GetChild( int i );
    XNodes::iterator GetChildIterator( LPXNode node );
    LPXNode CreateNode( LPCTSTR name = NULL, LPCTSTR value = NULL );
    LPXNode AppendChild( LPCTSTR name = NULL, LPCTSTR value = NULL );
    LPXNode AppendChild( LPXNode node );
    bool    RemoveChild( LPXNode node );
    LPXNode DetachChild( LPXNode node );

    // node/branch copy
    void    CopyNode( LPXNode node );
    void    CopyBranch( LPXNode branch );
    void    _CopyBranch( LPXNode node );
    LPXNode AppendChildBranch( LPXNode node );

    // modify attribute
    LPXAttr GetAttr( int i );
    XAttrs::iterator GetAttrIterator( LPXAttr node );
    LPXAttr CreateAttr( LPCTSTR anem = NULL, LPCTSTR value = NULL );
    LPXAttr AppendAttr( LPCTSTR name = NULL, LPCTSTR value = NULL );
    LPXAttr AppendAttr( LPXAttr attr );
    bool    RemoveAttr( LPXAttr attr );
    LPXAttr DetachAttr( LPXAttr attr );

    // operator overloads
    LPXNode operator [] ( int i ) { return GetChild(i); }
    XNode& operator = ( XNode& node ) { CopyBranch(&node); return *this; }

    _tagXMLNode() { parent = NULL; doc = NULL; type = XNODE_ELEMENT; }
    ~_tagXMLNode();

    void Close();
}XNode, *LPXNode;

// XMLDocument structure
typedef struct _tagXMLDocument : public XNode
{
    PARSEINFO   parse_info;

    _tagXMLDocument() { parent = NULL; doc = this; type = XNODE_DOC; }

    LPTSTR  Load( LPCTSTR pszXml, LPPARSEINFO pi = NULL );
    LPXNode GetRoot();

}XDoc, *LPXDoc;

// Helper Funtion
inline long XStr2Int( LPCTSTR str, long default_value = 0 )
{
    return ( str && *str ) ? _ttol(str) : default_value;
}

inline bool XIsEmptyString( LPCTSTR str )
{
    std::wstring s(str);
    CStringUtils::trim(s);

    return ( s.empty() || s == _T("") );
}
