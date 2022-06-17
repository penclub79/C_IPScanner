// XMLite.h: interface for the XMLite class.
//
// XMLite : XML Lite Parser Library
// by bro ( Cho,Kyung Min: bro@shinbiro.com ) 2002-10-30
// Microsoft MVP (Visual C++) bro@msmvp.com
// 
// History.
// 2002-10-29 : First Coded. Parsing XMLElelement and Attributes.
//              get xml parsed string ( looks good )
// 2002-10-30 : Get Node Functions, error handling ( not completed )
// 2002-12-06 : Helper Funtion string to long
// 2002-12-12 : Entity Helper Support
// 2003-04-08 : Close, 
// 2003-07-23 : add property escape_value. (now no escape on default)
// 2003-10-24 : bugfix) attribute parsing <tag a='1' \r\n/> is now ok
// 2004-03-05 : add branch copy functions
// 2004-06-14 : add _tcseistr/_tcsenistr/_tcsenicmp functions
// 2004-06-14 : now support, XML Document and PI, Comment, CDATA node
// 2004-06-15 : add value/ Find() functions
// 2004-06-15 : add force_parse : now can parse HTML (not-welformed xml)
// 
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLITE_H__786258A5_8360_4AE4_BDAF_2A52F8E1B877__INCLUDED_)
#define AFX_XMLITE_H__786258A5_8360_4AE4_BDAF_2A52F8E1B877__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	CHAR entity;					// entity ( & " ' < > )
	CHAR ref[10];					// entity reference ( &amp; &quot; etc )
	int ref_len;					// entity reference length
}XENTITY,*LPXENTITY;

typedef struct _tagXMLEntitys : public std::vector<XENTITY>
{
	LPXENTITY GetEntity( int entity );
	LPXENTITY GetEntity( LPSTR entity );	
	int GetEntityCount( LPCSTR str );
	int Ref2Entity( LPCSTR estr, LPSTR str, int strlen );
	int Entity2Ref( LPCSTR str, LPSTR estr, int estrlen );
	CStringA Ref2Entity( LPCSTR estr );
	CStringA Entity2Ref( LPCSTR str );	

	_tagXMLEntitys(){};
	_tagXMLEntitys( LPXENTITY entities, int count );
}XENTITYS,*LPXENTITYS;
extern XENTITYS entityDefault;
CStringA XRef2Entity( LPCSTR estr );
CStringA XEntity2Ref( LPCSTR str );	

typedef enum 
{
	PIE_PARSE_WELFORMED	= 0,
	PIE_ALONE_NOT_CLOSED,
	PIE_NOT_CLOSED,
	PIE_NOT_NESTED,
	PIE_ATTR_NO_VALUE
}PCODE;

// Parse info.
typedef struct _tagParseInfo
{
	bool		trim_value;			// [set] do trim when parse?
	bool		entity_value;		// [set] do convert from reference to entity? ( &lt; -> < )
	LPXENTITYS	entitys;			// [set] entity table for entity decode
	CHAR		escape_value;		// [set] escape value (default '\\')
	bool		force_parse;		// [set] force parse even if xml is not welformed

	LPSTR		xml;				// [get] xml source
	bool		erorr_occur;		// [get] is occurance of error?
	LPSTR		error_pointer;		// [get] error position of xml source
	PCODE		error_code;			// [get] error code
	CStringA		error_string;		// [get] error string

	LPXDoc		doc;
	_tagParseInfo() { trim_value = false; entity_value = true; force_parse = false; entitys = &entityDefault; xml = NULL; erorr_occur = false; error_pointer = NULL; error_code = PIE_PARSE_WELFORMED; escape_value = '\\'; }
}PARSEINFO,*LPPARSEINFO;
extern PARSEINFO piDefault;

// display optional environment
typedef struct _tagDispOption
{
	bool newline;			// newline when new tag
	bool reference_value;	// do convert from entity to reference ( < -> &lt; )
	char value_quotation_mark;	// val="" (default value quotation mark "
	LPXENTITYS	entitys;	// entity table for entity encode

	int tab_base;			// internal usage
	_tagDispOption() { newline = true; reference_value = true; entitys = &entityDefault; tab_base = 0; value_quotation_mark = '"'; }
}DISP_OPT, *LPDISP_OPT;
extern DISP_OPT optDefault;

// XAttr : Attribute Implementation
typedef struct _tagXMLAttr
{
	CStringA	name;
	CStringA	value;
	
	_tagXMLNode*	parent;

	CStringA GetXML( LPDISP_OPT opt = &optDefault );
}XAttr, *LPXAttr;

typedef enum
{
	XNODE_ELEMENT,				// general node '<element>...</element>' or <element/>
	XNODE_PI,					// <?xml version="1.0" ?>
	XNODE_COMMENT,				// <!-- comment -->
	XNODE_CDATA,				// <![CDATA[ cdata ]]>
	XNODE_DOC,					// internal virtual root
}NODE_TYPE;

// XMLNode structure
typedef struct _tagXMLNode
{
	// name and value
	CStringA name;
	CStringA	value;

	// internal variables
	LPXNode	parent;		// parent node
	XNodes	childs;		// child node
	XAttrs	attrs;		// attributes
	NODE_TYPE type;		// node type 
	LPXDoc	doc;		// document

	// Load/Save XML
	LPSTR	Load( LPCSTR pszXml, LPPARSEINFO pi = &piDefault );
	CStringA GetXML( LPDISP_OPT opt = &optDefault );
	CStringA GetText( LPDISP_OPT opt = &optDefault );

	// internal load functions
	LPSTR	LoadAttributes( LPCSTR pszAttrs, LPPARSEINFO pi = &piDefault );
	LPSTR	LoadAttributes( LPCSTR pszAttrs, LPCSTR pszEnd, LPPARSEINFO pi = &piDefault );
	LPSTR	LoadProcessingInstrunction( LPCSTR pszXml, LPPARSEINFO pi = &piDefault );
	LPSTR	LoadComment( LPCSTR pszXml, LPPARSEINFO pi = &piDefault ); 
	LPSTR	LoadCDATA( LPCSTR pszXml, LPPARSEINFO pi = &piDefault ); 

	// in own attribute list
	LPXAttr	GetAttr( LPCSTR attrname ); 
	LPCSTR	GetAttrValue( LPCSTR attrname ); 
	XAttrs	GetAttrs( LPCSTR name ); 

	// in one level child nodes
	LPXNode	GetChild( LPCSTR name ); 
	LPXNode	GetChildArg( LPCSTR name, ... ); 

	LPCSTR	GetChildValue( LPCSTR name ); 
	CStringA	GetChildText( LPCSTR name, LPDISP_OPT opt = &optDefault );
	XNodes	GetChilds( LPCSTR name ); 
	XNodes	GetChilds(); 

	LPXAttr GetChildAttr( LPCSTR name, LPCSTR attrname );
	LPCSTR GetChildAttrValue( LPCSTR name, LPCSTR attrname );
	
	// search node
	LPXNode	Find( LPCSTR name );

	// modify DOM 
	int		GetChildCount();
	LPXNode GetChild( int i );
	XNodes::iterator GetChildIterator( LPXNode node );
	LPXNode CreateNode( LPCSTR name = NULL, LPCSTR value = NULL );
	LPXNode	AppendChild( LPCSTR name = NULL, LPCSTR value = NULL );
	LPXNode	AppendChild( LPXNode node );
	bool	RemoveChild( LPXNode node );
	LPXNode DetachChild( LPXNode node );

	// node/branch copy
	void	CopyNode( LPXNode node );
	void	CopyBranch( LPXNode branch );
	void	_CopyBranch( LPXNode node );
	LPXNode	AppendChildBranch( LPXNode node );

	// modify attribute
	LPXAttr GetAttr( int i );
	XAttrs::iterator GetAttrIterator( LPXAttr node );
	LPXAttr CreateAttr( LPCSTR anem = NULL, LPCSTR value = NULL );
	LPXAttr AppendAttr( LPCSTR name = NULL, LPCSTR value = NULL );
	LPXAttr	AppendAttr( LPXAttr attr );
	bool	RemoveAttr( LPXAttr attr );
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
	PARSEINFO	parse_info;

	_tagXMLDocument() { parent = NULL; doc = this; type = XNODE_DOC; }
	
	LPSTR	Load( LPCSTR pszXml, LPPARSEINFO pi = NULL );
	LPXNode	GetRoot();

}XDoc, *LPXDoc;

// Helper Funtion
inline long XStr2Int( LPCSTR str, long default_value = 0 )
{
	return (str && *str) ? atol(str) : default_value;
}

inline bool XIsEmptyString( LPCSTR str )
{
	CStringA s(str);
	s.TrimLeft();
	s.TrimRight();

	return ( s.IsEmpty() || s == _T("") );
}

#endif // !defined(AFX_XMLITE_H__786258A5_8360_4AE4_BDAF_2A52F8E1B877__INCLUDED_)
