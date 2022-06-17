// XMLite.cpp: implementation of the XMLite class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLite.h"
#include <iostream>
#include <sstream>
#include <string>
#include <stdarg.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const CHAR chXMLTagOpen		= '<';
static const CHAR chXMLTagClose	= '>';
static const CHAR chXMLTagPre	= '/';
static const CHAR chXMLEscape = '\\';	// for value field escape

static const CHAR szXMLPIOpen[] = ("<?");
static const CHAR szXMLPIClose[] = ("?>");
static const CHAR szXMLCommentOpen[] = ("<!--");
static const CHAR szXMLCommentClose[] = ("-->");
static const CHAR szXMLCDATAOpen[] = ("<![CDATA[");
static const CHAR szXMLCDATAClose[] = ("]]>");


static const XENTITY x_EntityTable[] = {
		{ '&', ("&amp;"), 5 } ,
		{ '\"', ("&quot;"), 6 } ,
		{ '\'', ("&apos;"), 6 } ,
		{ '<', ("&lt;"), 4 } ,
		{ '>', ("&gt;"), 4 } 
	};

PARSEINFO piDefault;
DISP_OPT optDefault;
XENTITYS entityDefault((LPXENTITY)x_EntityTable, sizeof(x_EntityTable)/sizeof(x_EntityTable[0]) );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//========================================================
// Name   : _tcschrs
// Desc   : same with _tcspbrk 
// Param  :
// Return :
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPSTR _tcschrs( LPCSTR psz, LPCSTR pszchs )
{
	while( psz && *psz )
	{
		if( strchr( pszchs, *psz ) )
			return (LPSTR)psz;
		psz++;
	}
	return NULL;
}

//========================================================
// Name   : _tcsskip
// Desc   : skip space
// Param  : 
// Return : skiped string
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPSTR _tcsskip( LPCSTR psz )
{
	//while( psz && *psz == ' ' && *psz == 13 && *psz == 10 ) psz++;
	while( psz && isspace(*psz) ) psz++;
		
	return (LPSTR)psz;
}

//========================================================
// Name   : _tcsechr
// Desc   : similar with _tcschr with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPSTR _tcsechr( LPCSTR psz, int ch, int escape )
{
	LPSTR pch = (LPSTR)psz;

	while( pch && *pch )
	{
		if( escape != 0 && *pch == escape )
			pch++;
		else
		if( *pch == ch ) 
			return (LPSTR)pch;
		pch++;
	}
	return pch;
}

//========================================================
// Name   : _tcselen
// Desc   : similar with _tcslen with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
int _tcselen( int escape, LPSTR srt, LPSTR end = NULL ) 
{
	int len = 0;
	LPSTR pch = srt;
	if( end==NULL ) end = (LPSTR)sizeof(long);
	LPSTR prev_escape = NULL;
	while( pch && *pch && pch<end )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			len++;
		}
		pch++;
	}
	return len;
}

//========================================================
// Name   : _tcsecpy
// Desc   : similar with _tcscpy with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _tcsecpy( LPSTR psz, int escape, LPSTR srt, LPSTR end = NULL )
{
	LPSTR pch = srt;
	if( end==NULL ) end = (LPSTR)sizeof(long);
	LPSTR prev_escape = NULL;
	while( pch && *pch && pch<end )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			*psz++ = *pch;
		}

		pch++;
	}

	*psz = '\0';
}

//========================================================
// Name   : _tcsepbrk
// Desc   : similar with _tcspbrk with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPSTR _tcsepbrk( LPCSTR psz, LPCSTR chset, int escape )
{
	LPSTR pch = (LPSTR)psz;
	LPSTR prev_escape = NULL;
	while( pch && *pch )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			if( strchr( chset, *pch ) )
				return (LPSTR)pch;		
		}
		pch++;
	}
	return pch;
}

//========================================================
// Name   : _tcsenicmp
// Desc   : similar with _tcsnicmp with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
int _tcsenicmp( LPCSTR psz, LPCSTR str, int len, int escape )
{
	LPSTR pch = (LPSTR)psz;
	LPSTR prev_escape = NULL;
	LPSTR des = (LPSTR)str;
	int i = 0;
	
	while( pch && *pch && i < len )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			if( tolower(*pch) != tolower(des[i]) )
				break;
			i++;
		}
		pch ++;
	}
	
	// find
	if( i == len )
		return 0;
	if( psz[i] > des[i] )
		return 1;
	return -1;
}

//========================================================
// Name   : _tcsenistr
// Desc   : similar with _tcsistr with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPSTR _tcsenistr( LPCSTR psz, LPCSTR str, int len, int escape )
{
	LPSTR pch = (LPSTR)psz;
	LPSTR prev_escape = NULL;
	LPSTR des = (LPSTR)str;
	int i = 0;
	
	while( pch && *pch )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			if( _tcsenicmp( pch, str, len, escape ) == 0 )
				return (LPSTR)pch;
		}
		pch++;
	}
	return pch;
}

//========================================================
// Name   : _tcseistr
// Desc   : similar with _tcsistr with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPSTR _tcseistr( LPCSTR psz, LPCSTR str, int escape )
{
	int len = strlen( str );
	return _tcsenistr( psz, str, len, escape );
}

//========================================================
// Name   : _SetString
// Desc   : put string of (psz~end) on ps string
// Param  : trim - will be trim?
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _SetString( LPSTR psz, LPSTR end, CStringA* ps, bool trim = FALSE, int escape = 0 )
{
	//trim
	if( trim )
	{
		while( psz && psz < end && _istspace(*psz) ) psz++;
		while( (end-1) && psz < (end-1) && _istspace(*(end-1)) ) end--;
	}
	int len = end - psz;
	if( len <= 0 ) return;
	if( escape )
	{
		len = _tcselen( escape, psz, end );
		LPSTR pss = ps->GetBufferSetLength( len );
		_tcsecpy( pss, escape, psz, end );
	}
	else
	{
		LPSTR pss = ps->GetBufferSetLength(len + 1 );
		memcpy( pss, psz, len );
		pss[len] = '\0';
	}
}

_tagXMLNode::~_tagXMLNode()
{
	Close();
}

void _tagXMLNode::Close()
{
	for (int i = 0; i < (int)childs.size(); i++)
	{
		LPXNode p = childs[i];
		if( p )
		{
			delete p; childs[i] = NULL;
		}
	}
	childs.clear();
	
	for (int i = 0; i < (int)attrs.size(); i++)
	{
		LPXAttr p = attrs[i];
		if( p )
		{
			delete p; attrs[i] = NULL;
		}
	}
	attrs.clear();
}
	
// attr1="value1" attr2='value2' attr3=value3 />
//                                            ^- return pointer
//========================================================
// Name   : LoadAttributes
// Desc   : loading attribute plain xml text
// Param  : pszAttrs - xml of attributes
//          pi = parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPSTR _tagXMLNode::LoadAttributes( LPCSTR pszAttrs , LPPARSEINFO pi /*= &piDefault*/)
{
	LPSTR xml = (LPSTR)pszAttrs;

	while( xml && *xml )
	{
		if (xml = _tcsskip(xml))
		{
			// close tag
			if( *xml == chXMLTagClose || *xml == chXMLTagPre )
				// wel-formed tag
				return xml;

			// XML Attr Name
			CHAR* pEnd = strpbrk( xml, " =" );
			if( pEnd == NULL ) 
			{
				// error
				if( pi->erorr_occur == false ) 
				{
					pi->erorr_occur = true;
					pi->error_pointer = xml;
					pi->error_code = PIE_ATTR_NO_VALUE;
					pi->error_string.Format(("<%s> attribute has error "), name);
				}
				return NULL;
			}
			
			LPXAttr attr = new XAttr;
			attr->parent = this;

			// XML Attr Name
			_SetString( xml, pEnd, &attr->name );
			
			// add new attribute
			attrs.push_back( attr );
			xml = pEnd;
			
			// XML Attr Value
			if (xml = _tcsskip(xml))
			{
				//if( xml = strchr( xml, '=' ) )
				if( *xml == '=' )
				{
					if (xml = _tcsskip(++xml))
					{
						// if " or '
						// or none quote
						int quote = *xml;
						if( quote == '"' || quote == '\'' )
							pEnd = _tcsechr(++xml, quote, chXMLEscape);
						else
						{
							//attr= value> 
							// none quote mode
							//pEnd = strechr( xml, ' ', '\\' );
							pEnd = _tcsepbrk(xml, (" >"), chXMLEscape);
						}

						bool trim = pi->trim_value;
						CHAR escape = pi->escape_value;
						//_SetString( xml, pEnd, &attr->value, trim, chXMLEscape );	
						_SetString( xml, pEnd, &attr->value, trim, escape );
						xml = pEnd;
						// ATTRVALUE 
						if( pi->entity_value && pi->entitys )
							attr->value = pi->entitys->Ref2Entity(attr->value);

						if( quote == '"' || quote == '\'' )
							xml++;
					}
				}
			}
		}
	}

	// not wel-formed tag
	return NULL;
}

// attr1="value1" attr2='value2' attr3=value3 />
//                                            ^- return pointer
//========================================================
// Name   : LoadAttributes
// Desc   : loading attribute plain xml text
// Param  : pszAttrs - xml of attributes
//          pszEnd - last string
//          pi = parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPSTR _tagXMLNode::LoadAttributes( LPCSTR pszAttrs, LPCSTR pszEnd, LPPARSEINFO pi /*= &piDefault*/ )
{
	LPSTR xml = (LPSTR)pszAttrs;

	while( xml && *xml )
	{
		if (xml = _tcsskip(xml))
		{
			// close tag
			if( xml >= pszEnd )
				// wel-formed tag
				return xml;

			// XML Attr Name
			CHAR* pEnd = strpbrk(xml, " =");
			if( pEnd == NULL ) 
			{
				// error
				if( pi->erorr_occur == false ) 
				{
					pi->erorr_occur = true;
					pi->error_pointer = xml;
					pi->error_code = PIE_ATTR_NO_VALUE;
					pi->error_string.Format(("<%s> attribute has error "), name);
				}
				return NULL;
			}
			
			LPXAttr attr = new XAttr;
			attr->parent = this;

			// XML Attr Name
			_SetString( xml, pEnd, &attr->name );
			
			// add new attribute
			attrs.push_back( attr );
			xml = pEnd;
			
			// XML Attr Value
			if (xml = _tcsskip(xml))
			{
				//if( xml = strchr( xml, '=' ) )
				if( *xml == '=' )
				{
					if (xml = _tcsskip(++xml))
					{
						// if " or '
						// or none quote
						int quote = *xml;
						if( quote == '"' || quote == '\'' )
							pEnd = _tcsechr(++xml, quote, chXMLEscape);
						else
						{
							//attr= value> 
							// none quote mode
							//pEnd = strechr( xml, ' ', '\\' );
							pEnd = _tcsepbrk(xml, (" >"), chXMLEscape);
						}

						bool trim = pi->trim_value;
						CHAR escape = pi->escape_value;
						//_SetString( xml, pEnd, &attr->value, trim, chXMLEscape );	
						_SetString( xml, pEnd, &attr->value, trim, escape );
						xml = pEnd;
						// ATTRVALUE 
						if( pi->entity_value && pi->entitys )
							attr->value = pi->entitys->Ref2Entity(attr->value);

						if( quote == '"' || quote == '\'' )
							xml++;
					}
				}
			}
		}
	}

	// not wel-formed tag
	return NULL;
}

// <?xml version="1.0"?>
//                      ^- return pointer
//========================================================
// Name   : LoadProcessingInstrunction
// Desc   : loading processing instruction
// Param  : pszXml - PI string
//          pi - parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPSTR _tagXMLNode::LoadProcessingInstrunction( LPCSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	// find the end of pi
	LPSTR end = _tcsenistr( pszXml, szXMLPIClose, sizeof(szXMLPIClose)-1, pi ? pi->escape_value : 0 );
	if( end == NULL )
		return NULL;

	// process pi
	if( doc )
	{
		LPSTR xml = (LPSTR)pszXml;

		LPXNode node = new XNode;
		node->parent = this;
		node->doc = doc;
		node->type = XNODE_PI;
		
		xml += sizeof(szXMLPIOpen)-1;
		CHAR* pTagEnd = strpbrk( xml, " ?>" );
		_SetString( xml, pTagEnd, &node->name );
		xml = pTagEnd;
		
		node->LoadAttributes( xml, end, pi );

		doc->childs.push_back( node );
	}

	end += sizeof(szXMLPIClose)-1;
	return end;
}

// <!-- comment -->
//                 ^- return pointer
//========================================================
// Name   : LoadComment
// Desc   : loading comment
// Param  : pszXml - comment string
//          pi - parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPSTR _tagXMLNode::LoadComment( LPCSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	// find the end of comment
	LPSTR end = _tcsenistr( pszXml, szXMLCommentClose, sizeof(szXMLCommentClose)-1, pi ? pi->escape_value : 0 );
	if( end == NULL )
		return NULL;

	// process comment
	LPXNode par = parent;
	if( parent == NULL && doc )
		par = (LPXNode)&doc;
	if( par )
	{
		LPSTR xml = (LPSTR)pszXml;
		xml += sizeof(szXMLCommentOpen)-1;
		
		LPXNode node = new XNode;
		node->parent = this;
		node->doc = doc;
		node->type = XNODE_COMMENT;
		node->name = ("#COMMENT");
		_SetString( xml, end, &node->value, FALSE );

		par->childs.push_back( node );
	}

	end += sizeof(szXMLCommentClose)-1;
	return end;
}

// <![CDATA[ cdata ]]>
//                    ^- return pointer
//========================================================
// Name   : LoadCDATA
// Desc   : loading CDATA
// Param  : pszXml - CDATA string
//          pi - parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPSTR _tagXMLNode::LoadCDATA( LPCSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	// find the end of CDATA
	LPSTR end = _tcsenistr( pszXml, szXMLCDATAClose, sizeof(szXMLCDATAClose)-1, pi ? pi->escape_value : 0 );
	if( end == NULL )
		return NULL;

	// process CDATA
	LPXNode par = parent;
	if( parent == NULL && doc )
		par = (LPXNode)&doc;
	if( par )
	{
		LPSTR xml = (LPSTR)pszXml;
		xml += sizeof(szXMLCDATAOpen)-1;
		
		LPXNode node = new XNode;
		node->parent = this;
		node->doc = doc;
		node->type = XNODE_CDATA;
		node->name = ("#CDATA");
		_SetString( xml, end, &node->value, FALSE );

		par->childs.push_back( node );
	}

	end += sizeof(szXMLCDATAClose)-1;
	return end;
}

//========================================================
// Name   : LoadOtherNodes
// Desc   : internal function for loading PI/CDATA/Comment
// Param  : node - current xml node
//          pbRet - error occur
//          pszXml - CDATA string
//          pi - parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPSTR LoadOtherNodes( LPXNode node, bool* pbRet, LPCSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	LPSTR xml = (LPSTR)pszXml;
	bool do_other_type = true;
	*pbRet = false;

	while( xml && do_other_type )
	{
		do_other_type = false;

		xml = _tcsskip( xml );
		LPSTR prev = xml;
		// is PI( Processing Instruction ) Node?
		if( _strnicmp( xml, szXMLPIOpen, sizeof(szXMLPIOpen)-1 ) == 0 )
		{
			// processing instrunction parse
			// return pointer is next node of pi
			xml = node->LoadProcessingInstrunction( xml, pi );
			//if( xml == NULL )
			//	return NULL;
			// restart xml parse
		}

		if( xml != prev )
			do_other_type = true;
		xml = _tcsskip( xml );
		prev = xml;

		// is comment Node?
		if (_strnicmp(xml, szXMLCommentOpen, sizeof(szXMLCommentOpen) - 1) == 0)
		{
			// processing comment parse
			// return pointer is next node of comment
			xml = node->LoadComment( xml, pi );
			// comment node is terminal node
			if( node->parent && node->parent->type != XNODE_DOC 
				&& xml != prev )
			{
				*pbRet = true;
				return xml;
			}
			// restart xml parse when this node is root doc node
		}

		if( xml != prev )
			do_other_type = true;

		xml = _tcsskip( xml );
		prev = xml;
		// is CDATA Node?
		if (_strnicmp(xml, szXMLCDATAOpen, sizeof(szXMLCDATAOpen) - 1) == 0)
		{
			// processing CDATA parse
			// return pointer is next node of CDATA
			xml = node->LoadCDATA( xml, pi );
			// CDATA node is terminal node
			if( node->parent && node->parent->type != XNODE_DOC 
				&& xml != prev )
			{
				*pbRet = true;
				return xml;
			}
			// restart xml parse when this node is root doc node
		}

		if( xml != prev )
			do_other_type = true;
	}

	return xml;
}

// <TAG attr1="value1" attr2='value2' attr3=value3 >
// </TAG>
// or
// <TAG />
//        ^- return pointer
//========================================================
// Name   : Load
// Desc   : load xml plain text
// Param  : pszXml - plain xml text
//          pi = parser information
// Return : advanced string pointer  (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPSTR _tagXMLNode::Load( LPCSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	// Close it
	Close();

	LPSTR xml = (LPSTR)pszXml;

	xml = strchr(xml, chXMLTagOpen);
	if( xml == NULL )
		return NULL;

	// Close Tag
	if( *(xml+1) == chXMLTagPre ) // </Close
		return xml;

	// Load Other Node before <Tag>(pi, comment, CDATA etc)
	bool bRet = false;
	LPSTR ret = NULL;
	ret = LoadOtherNodes( this, &bRet, xml, pi );
	if( ret != NULL ) 
		xml = ret;
	if( bRet ) 
		return xml;

	// XML Node Tag Name Open
	xml++;
	CHAR* pTagEnd = strpbrk(xml, " />\r\n");
	_SetString( xml, pTagEnd, &name );
	xml = pTagEnd;
	// Generate XML Attributte List
	if( xml = LoadAttributes( xml, pi ) )
	{
		// alone tag <TAG ... />
		if( *xml == chXMLTagPre )
		{
			xml++;
			if( *xml == chXMLTagClose )
				// wel-formed tag
				return ++xml;
			else
			{
				// error: <TAG ... / >
				if( pi->erorr_occur == false ) 
				{
					pi->erorr_occur = true;
					pi->error_pointer = xml;
					pi->error_code = PIE_ALONE_NOT_CLOSED;
					pi->error_string = ("Element must be closed.");
				}
				// not wel-formed tag
				return NULL;
			}
		}
		else
		// open/close tag <TAG ..> ... </TAG>
		//                        ^- current pointer
		{
			// if text value is not exist, then assign value
			//if( this->value.IsEmpty() || this->value == ("") )
			if( XIsEmptyString( value ) )
			{
				// Text Value 
				CHAR* pEnd = _tcsechr( ++xml, chXMLTagOpen, chXMLEscape );
				if( pEnd == NULL ) 
				{
					if( pi->erorr_occur == false ) 
					{
						pi->erorr_occur = true;
						pi->error_pointer = xml;
						pi->error_code = PIE_NOT_CLOSED;
						pi->error_string.Format(("%s must be closed with </%s>"), name);
					}
					// error cos not exist CloseTag </TAG>
					return NULL;
				}
				
				bool trim = pi->trim_value;
				CHAR escape = pi->escape_value;
				//_SetString( xml, pEnd, &value, trim, chXMLEscape );
				_SetString( xml, pEnd, &value, trim, escape );

				xml = pEnd;
				// TEXTVALUE reference
				if( pi->entity_value && pi->entitys )
					value = pi->entitys->Ref2Entity(value);
			}

			// generate child nodes
			while( xml && *xml )
			{
				LPXNode node = new XNode;
				node->parent = this;
				node->doc = doc;
				node->type = type;
				
				xml = node->Load( xml,pi );
				if( node->name.IsEmpty() == FALSE )
				{
					childs.push_back( node );

				}
				else
				{
					delete node;
				}

				// open/close tag <TAG ..> ... </TAG>
				//                             ^- current pointer
				// CloseTag case
				if( xml && *xml && *(xml+1) && *xml == chXMLTagOpen && *(xml+1) == chXMLTagPre )
				{
					// </Close>
					xml+=2; // C
					
					if( xml = _tcsskip( xml ) )
					{
						CStringA closename;
						CHAR* pEnd = strpbrk(xml, " >");
						if( pEnd == NULL ) 
						{
							if( pi->erorr_occur == false ) 
							{
								pi->erorr_occur = true;
								pi->error_pointer = xml;
								pi->error_code = PIE_NOT_CLOSED;
								pi->error_string.Format(("it must be closed with </%s>"), name);
							}
							// error
							return NULL;
						}
						_SetString( xml, pEnd, &closename );
						if( closename == this->name )
						{
							// wel-formed open/close
							xml = pEnd+1;
							// return '>' or ' ' after pointer
							return xml;
						}
						else
						{
							xml = pEnd+1;
							// 2004.6.15 - example <B> alone tag
							// now it can parse with attribute 'force_arse'
							if( pi->force_parse == false )
							{
								// not welformed open/close
								if( pi->erorr_occur == false ) 
								{
									pi->erorr_occur = true;
									pi->error_pointer = xml;
									pi->error_code = PIE_NOT_NESTED;
									pi->error_string.Format(("'<%s> ... </%s>' is not wel-formed."), name, closename);
								}
								return NULL;
							}
						}
					}
				}
				else	// Alone child Tag Loaded
						// else ﾇﾘｾﾟﾇﾏｴﾂﾁ・ｸｻｾﾆｾﾟﾇﾏｴﾂﾁ・ﾀﾇｽﾉｰ｣ｴﾙ.
				{
					
					//if( xml && this->value.IsEmpty() && *xml !=chXMLTagOpen )
					if( xml && XIsEmptyString( value ) && *xml !=chXMLTagOpen )
					{
						// Text Value 
						CHAR* pEnd = _tcsechr( xml, chXMLTagOpen, chXMLEscape );
						if( pEnd == NULL ) 
						{
							// error cos not exist CloseTag </TAG>
							if( pi->erorr_occur == false )  
							{
								pi->erorr_occur = true;
								pi->error_pointer = xml;
								pi->error_code = PIE_NOT_CLOSED;
								pi->error_string.Format(("it must be closed with </%s>"), name);
							}
							return NULL;
						}
						
						bool trim = pi->trim_value;
						CHAR escape = pi->escape_value;
						//_SetString( xml, pEnd, &value, trim, chXMLEscape );
						_SetString( xml, pEnd, &value, trim, escape );

						xml = pEnd;
						//TEXTVALUE
						if( pi->entity_value && pi->entitys )
							value = pi->entitys->Ref2Entity(value);
					}
				}
			}
		}
	}

	return xml;
}

// <?xml version='1.0'?>
// <TAG attr1="value1" attr2='value2' attr3=value3 >
// </TAG>
// or
// <TAG />
//        ^- return pointer
//========================================================
// Name   : Load
// Desc   : load xml plain text for xml document
// Param  : pszXml - plain xml text
//          pi = parser information
// Return : advanced string pointer  (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPSTR _tagXMLDocument::Load( LPCSTR pszXml, LPPARSEINFO pi /*= NULL*/ )
{
	LPXNode node = new XNode;
	node->parent = (LPXNode)this;
	node->type = XNODE_ELEMENT;
	node->doc = this;
	LPSTR end;
	
	if( pi == NULL )
		pi = &parse_info;

	if( (end = node->Load( pszXml, pi )) == NULL )
	{
		delete node;
		return NULL;
	}

	childs.push_back( node );

	// Load Other Node after </Tag>(pi, comment, CDATA etc)
	LPSTR ret;
	bool bRet = false;
	ret = LoadOtherNodes( node, &bRet, end, pi );
	if( ret != NULL ) 
		end = ret;

	return end;
}

LPXNode	_tagXMLDocument::GetRoot()
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end() ; ++(it) )
	{
		LPXNode node = *it;
		if( node->type == XNODE_ELEMENT )
			return node;
	}
	return NULL;
}

//========================================================
// Name   : GetXML
// Desc   : convert plain xml text from parsed xml attirbute
// Param  :
// Return : converted plain string
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
CStringA _tagXMLAttr::GetXML( LPDISP_OPT opt /*= &optDefault*/ )
{
	std::ostringstream os;
	//os << (LPCSTR)name << "='" << (LPCSTR)value << "' ";
	
	os << (LPCSTR)name << "=" << (char)opt->value_quotation_mark 
		<< (LPCSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value):value) 
		<< (char)opt->value_quotation_mark << " ";
	return os.str().c_str();
}

//========================================================
// Name   : GetXML
// Desc   : convert plain xml text from parsed xml node
// Param  :
// Return : converted plain string
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
CStringA _tagXMLNode::GetXML( LPDISP_OPT opt /*= &optDefault*/ )
{
	std::ostringstream os;

	// tab
	if( opt && opt->newline )
	{
		os << "\r\n";
		for( int i = 0 ; i < opt->tab_base ; i++)
			os << " \t ";
	}

	if( type == XNODE_DOC )
	{
		for (int i = 0; i < (int)childs.size(); i++)
			os << (LPCSTR)childs[i]->GetXML( opt );
		return os.str().c_str();
	}
	else
	if( type == XNODE_PI )
	{
		// <?TAG
		os << szXMLPIOpen << (LPCSTR)name;
		// <?TAG Attr1="Val1" 
		if( attrs.empty() == false ) os << ' ';
		for (int i = 0; i < (int)attrs.size(); i++)
		{
			os << (LPCSTR)attrs[i]->GetXML(opt);
		}
		//?>
		os << szXMLPIClose;	
		return os.str().c_str();
	}
	else
	if( type == XNODE_COMMENT )
	{
		// <--comment
		os << szXMLCommentOpen << (LPCSTR)value;
		//-->
		os << szXMLCommentClose;	
		return os.str().c_str();
	}
	else
	if( type == XNODE_CDATA )
	{
		// <--comment
		os << szXMLCDATAOpen << (LPCSTR)value;
		//-->
		os << szXMLCDATAClose;	
		return os.str().c_str();
	}

	// <TAG
	os << '<' << (LPCSTR)name;

	// <TAG Attr1="Val1" 
	if( attrs.empty() == false )
		os << ' ';

	for (int i = 0; i < (int)attrs.size(); i++)
	{
		os << (LPCSTR)attrs[i]->GetXML(opt);
	}
	
	if( childs.empty() && value.IsEmpty() )
	{
		// <TAG Attr1="Val1"/> alone tag 
		os << "/>";	
	}
	else
	{
		// <TAG Attr1="Val1"> and get child
		os << '>';
		if( opt && opt->newline && !childs.empty() )
		{
			opt->tab_base++;
		}

		for (int i = 0; i < (int)childs.size(); i++)
			os << (LPCSTR)childs[i]->GetXML( opt );
		
		// Text Value
		if (value != (""))
		{
			if( opt && opt->newline && !childs.empty() )
			{
				if( opt && opt->newline )
				{
					os << "\r\n";
					for( int i = 0 ; i < opt->tab_base ; i++)
						os << " \t ";
				}
			}

			os << (LPCSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value):value);
		}

		// </TAG> CloseTag
		if( opt && opt->newline && !childs.empty() )
		{
			os << "\r\n";
			for( int i = 0 ; i < opt->tab_base-1 ; i++)
				os << " \t ";
		}

		os << "</" << (LPCSTR)name << '>';

		if( opt && opt->newline )
		{
			if( !childs.empty() )
				opt->tab_base--;
		}
	}
	

	return os.str().c_str();
}

//========================================================
// ﾇﾔｼ・: GetText
// ｼｳ  ｸ・: ｳ・・ﾇﾏｳｪｸｦ ﾅﾘｽｺﾆｮ ｹｮﾀﾚｿｭｷﾎ ｹﾝﾈｯ
// ﾀﾎ  ﾀﾚ :
// ｸｮﾅﾏｰｪ : ｺｯﾈｯｵﾈ ｹｮﾀﾚｿｭ
//--------------------------------------------------------
// ﾀﾛｼｺﾀﾚ   ﾀﾛｼｺﾀﾏ                 ﾀﾛｼｺﾀﾌﾀｯ
// ﾁｶｰ貉ﾎ   2004-06-15
//========================================================
CStringA _tagXMLNode::GetText( LPDISP_OPT opt /*= &optDefault*/ )
{
	std::ostringstream os;

	if( type == XNODE_DOC )
	{
		for (int i = 0; i < (int)childs.size(); i++)
			os << (LPCSTR)childs[i]->GetText( opt );
	}
	else
	if( type == XNODE_PI )
	{
		// no text
	}
	else
	if( type == XNODE_COMMENT )
	{
		// no text
	}
	else
	if( type == XNODE_CDATA )
	{
		os << (LPCSTR)value;
	}
	else
	if( type == XNODE_ELEMENT )
	{
		if( childs.empty() && value.IsEmpty() )
		{
			// no text
		}
		else
		{
			// childs text
			for (int i = 0; i < (int)childs.size(); i++)
				os << (LPCSTR)childs[i]->value;
			
			// Text Value
			os << (LPCSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value):value);
		}
	}
	
	return os.str().c_str();
}

//========================================================
// Name   : GetAttr
// Desc   : get attribute with attribute name
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr	_tagXMLNode::GetAttr( LPCSTR attrname )
{
	for (int i = 0; i < (int)attrs.size(); i++)
	{
		LPXAttr attr = attrs[i];
		if( attr )
		{
			if( attr->name == attrname )
				return attr;
		}
	}
	return NULL;
}

//========================================================
// Name   : GetAttrs
// Desc   : find attributes with attribute name, return its list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XAttrs _tagXMLNode::GetAttrs( LPCSTR name )
{
	XAttrs attrs;
	for (int i = 0; i < (int)attrs.size(); i++)
	{
		LPXAttr attr = attrs[i];
		if( attr )
		{
			if( attr->name == name )
				attrs.push_back( attr );
		}
	}
	return attrs;
}

//========================================================
// Name   : GetAttrValue
// Desc   : get attribute with attribute name, return its value
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPCSTR	_tagXMLNode::GetAttrValue( LPCSTR attrname )
{
	LPXAttr attr = GetAttr( attrname );
	return attr ? (LPCSTR)attr->value : NULL;
}

XNodes _tagXMLNode::GetChilds()
{
	return childs;
}

//========================================================
// Name   : GetChilds
// Desc   : Find childs with name and return childs list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XNodes _tagXMLNode::GetChilds( LPCSTR name )
{
	XNodes nodes;
	for( int i = 0 ; i < (int)childs.size(); i++ )
	{
		LPXNode node = childs[i];
		if( node )
		{
			if( node->name == name )
				nodes.push_back( node );
		}
	}
	return nodes;	
}

//========================================================
// Name   : GetChild
// Desc   : get child node with index
// Param  :
// Return : NULL return if no child.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::GetChild( int i )
{
	if (i >= 0 && i < (int)childs.size())
		return childs[i];
	return NULL;
}

//========================================================
// Name   : GetChildCount
// Desc   : get child node count
// Param  :
// Return : 0 return if no child
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-12-26
//========================================================
int	_tagXMLNode::GetChildCount()
{
	return childs.size();
}

//========================================================
// Name   : GetChild
// Desc   : Find child with name and return child
// Param  :
// Return : NULL return if no child.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::GetChild( LPCSTR name )
{
	for (int i = 0; i < (int)childs.size(); i++)
	{
		LPXNode node = childs[i];
		if( node )
		{
			if( node->name == name )
				return node;
		}
	}
	return NULL;
}

LPXNode	_tagXMLNode::GetChildArg( LPCSTR name, ... )
{
	const char*	pReturn			= name;
	INT			iStringLength	= 0;
	CStringA	strTemp;
	va_list     va				;
	LPXNode		lpChildNode		= NULL;
	LPXNode		lpChildTemp		= NULL;

	va_start(va, name);
	strTemp	= name;
	strTemp.TrimRight();

	while( 0 < strTemp.GetLength() )
	{
		lpChildTemp = Find( strTemp );

		if( lpChildTemp )
		{
			pReturn	= va_arg( va, const char* );
			strTemp	= pReturn;
			strTemp.TrimRight();

			lpChildNode	= lpChildTemp;
		}
		else
		{
			break;
		}
	}

	va_end(va);

	if( lpChildNode )
	{
		return lpChildNode;
	}

	return NULL;
}

//========================================================
// Name   : GetChildValue
// Desc   : Find child with name and return child's value
// Param  :
// Return : NULL return if no child.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPCSTR	_tagXMLNode::GetChildValue( LPCSTR name )
{
	LPXNode node = GetChild( name );
	return (node != NULL)? (LPCSTR)node->value : NULL;
}

CStringA	_tagXMLNode::GetChildText( LPCSTR name, LPDISP_OPT opt /*= &optDefault*/ )
{
	LPXNode node = GetChild( name );
	return (node != NULL) ? node->GetText(opt) : ("");
}

LPXAttr _tagXMLNode::GetChildAttr( LPCSTR name, LPCSTR attrname )
{
	LPXNode node = GetChild(name);
	return node ? node->GetAttr(attrname) : NULL;
}

LPCSTR _tagXMLNode::GetChildAttrValue( LPCSTR name, LPCSTR attrname )
{
	LPXAttr attr = GetChildAttr( name, attrname );
	return attr ? (LPCSTR)attr->value : NULL;
}

//========================================================
// Name   : Find
// Desc   : find node with tag name from it's all childs
// Param  :
// Return : NULL return if no found node.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::Find( LPCSTR name )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end(); ++(it))
	{
		LPXNode child = *it;
		if( child->name == name )
			return child;

		XNodes::iterator it = child->childs.begin();
		for( ; it != child->childs.end(); ++(it))
		{
			LPXNode find = child->Find( name );
			if( find != NULL )
				return find;
		}
	}

	return NULL;
}

//========================================================
// Name   : GetChildIterator
// Desc   : get child nodes iterator
// Param  :
// Return : NULL return if no childs.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XNodes::iterator _tagXMLNode::GetChildIterator( LPXNode node )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end() ; ++(it) )
	{
		if( *it == node )
			return it;
	}
	return it;
}

//========================================================
// Name   : AppendChild
// Desc   : add node
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::AppendChild( LPCSTR name /*= NULL*/, LPCSTR value /*= NULL*/ )
{
	return AppendChild( CreateNode( name, value ) );
}

//========================================================
// Name   : AppendChild
// Desc   : add node
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::AppendChild( LPXNode node )
{
	node->parent = this;
	node->doc = doc;
	childs.push_back( node );
	return node;
}

//========================================================
// Name   : RemoveChild
// Desc   : detach node and delete object
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
bool _tagXMLNode::RemoveChild( LPXNode node )
{
	XNodes::iterator it = GetChildIterator( node );
	if (it._Ptr)
	{
		delete *it;
		childs.erase( it );
		return true;
	}
	return false;
}

//========================================================
// Name   : GetAttr
// Desc   : get attribute with index in attribute list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::GetAttr( int i )
{
	if (i >= 0 && i < (int)attrs.size())
		return attrs[i];
	return NULL;
}

//========================================================
// Name   : GetAttrIterator
// Desc   : get attribute iterator
// Param  : 
// Return : std::vector<LPXAttr>::iterator
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XAttrs::iterator _tagXMLNode::GetAttrIterator( LPXAttr attr )
{
	XAttrs::iterator it = attrs.begin();
	for( ; it != attrs.end() ; ++(it) )
	{
		if( *it == attr )
			return it;
	}

	return it;
}

//========================================================
// Name   : AppendAttr
// Desc   : add attribute
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::AppendAttr( LPXAttr attr )
{
	attr->parent = this;
	attrs.push_back( attr );
	return attr;
}

//========================================================
// Name   : RemoveAttr
// Desc   : detach attribute and delete object
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
bool _tagXMLNode::RemoveAttr( LPXAttr attr )
{
	XAttrs::iterator it = GetAttrIterator( attr );
	if( it._Ptr )
	{
		delete *it;
		attrs.erase( it );
		return true;
	}
	return false;
}

//========================================================
// Name   : CreateNode
// Desc   : Create node object and return it
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::CreateNode( LPCSTR name /*= NULL*/, LPCSTR value /*= NULL*/ )
{
	LPXNode node = new XNode;
	node->name = name;
	node->value = value;
	return node;
}

//========================================================
// Name   : CreateAttr
// Desc   : create Attribute object and return it
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::CreateAttr( LPCSTR name /*= NULL*/, LPCSTR value /*= NULL*/ )
{
	LPXAttr attr = new XAttr;
	attr->name = name;
	attr->value = value;
	return attr;
}

//========================================================
// Name   : AppendAttr
// Desc   : add attribute
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::AppendAttr( LPCSTR name /*= NULL*/, LPCSTR value /*= NULL*/ )
{
	return AppendAttr( CreateAttr( name, value ) );
}

//========================================================
// Name   : DetachChild
// Desc   : no delete object, just detach in list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::DetachChild( LPXNode node )
{
	XNodes::iterator it = GetChildIterator( node );
	if (it._Ptr)
	{
		childs.erase( it );
		return node;
	}
	return NULL;
}

//========================================================
// Name   : DetachAttr
// Desc   : no delete object, just detach in list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::DetachAttr( LPXAttr attr )
{
	XAttrs::iterator it = GetAttrIterator( attr );
	if( it._Ptr )
	{
		attrs.erase( it );
		return attr;
	}
	return NULL;
}

//========================================================
// Name   : CopyNode
// Desc   : copy current level node with own attributes
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _tagXMLNode::CopyNode( LPXNode node )
{
	Close();

	doc = node->doc;
	parent = node->parent;
	name = node->name;
	value = node->value;
	type = node->type;

	// copy attributes
	for (int i = 0; i < (int)node->attrs.size(); i++)
	{
		LPXAttr attr = node->attrs[i];
		if( attr )
			AppendAttr( attr->name, attr->value );
	}
}

//========================================================
// Name   : _CopyBranch
// Desc   : recursive internal copy branch 
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _tagXMLNode::_CopyBranch( LPXNode node )
{
	CopyNode( node );

	for (int i = 0; i < (int)node->childs.size(); i++)
	{
		LPXNode child = node->childs[i];
		if( child )
		{
			LPXNode mychild = new XNode;
			mychild->CopyNode( child );
			AppendChild( mychild );

			mychild->_CopyBranch( child );
		}
	}
}

//========================================================
// Name   : AppendChildBranch
// Desc   : add child branch ( deep-copy )
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::AppendChildBranch( LPXNode node )
{
	LPXNode child = new XNode;
	child->CopyBranch( node );

	return AppendChild( child );
}

//========================================================
// Name   : CopyBranch
// Desc   : copy branch ( deep-copy )
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _tagXMLNode::CopyBranch( LPXNode branch )
{
	Close();
	
	_CopyBranch( branch );
}


_tagXMLEntitys::_tagXMLEntitys( LPXENTITY entities, int count )
{
	for( int i = 0; i < count; i++)
		push_back( entities[i] );
}

LPXENTITY _tagXMLEntitys::GetEntity( int entity )
{
	for (int i = 0; i < (int)size(); i++)
	{
		if( at(i).entity == entity )
			return LPXENTITY(&at(i));
	}
	return NULL;
}

LPXENTITY _tagXMLEntitys::GetEntity( LPSTR entity )
{
	for (int i = 0; i < (int)size(); i++)
	{
		LPSTR ref = (LPSTR)at(i).ref;
		LPSTR ps = entity;
		while( ref && *ref )
			if( *ref++ != *ps++ )
				break;
		if( ref && !*ref )	// found!
			return LPXENTITY(&at(i));
	}
	return NULL;
}

int _tagXMLEntitys::GetEntityCount( LPCSTR str )
{
	int nCount = 0;
	LPSTR ps = (LPSTR)str;
	while( ps && *ps )
		if( GetEntity( *ps++ ) ) nCount ++;
	return nCount;
}

int _tagXMLEntitys::Ref2Entity( LPCSTR estr, LPSTR str, int strlen )
{
	LPSTR pes = (LPSTR)estr;
	LPSTR ps = str;
	LPSTR ps_end = ps+strlen;
	while( pes && *pes && ps < ps_end )
	{
		LPXENTITY ent = GetEntity( pes );
		if( ent )
		{
			// copy entity meanning char
			*ps = ent->entity;
			pes += ent->ref_len;
		}
		else
			*ps = *pes++;	// default character copy
		ps++;
	}
	*ps = '\0';
	
	// total copied characters
	return ps-str;	
}

int _tagXMLEntitys::Entity2Ref( LPCSTR str, LPSTR estr, int estrlen )
{
	LPSTR ps = (LPSTR)str;
	LPSTR pes = (LPSTR)estr;
	LPSTR pes_end = pes+estrlen;
	while( ps && *ps && pes < pes_end )
	{
		LPXENTITY ent = GetEntity( *ps );
		if( ent )
		{
			// copy entity string
			LPSTR ref = (LPSTR)ent->ref;
			while( ref && *ref )
				*pes++ = *ref++;
		}
		else
			*pes++ = *ps;	// default character copy
		ps++;
	}
	*pes = '\0';
	
	// total copied characters
	return pes-estr;
}

CStringA _tagXMLEntitys::Ref2Entity( LPCSTR estr )
{
	CStringA es;
	if( estr )
	{
		int len = strlen(estr);
		LPSTR esbuf = es.GetBufferSetLength( len+1 );
		if( esbuf )
			Ref2Entity( estr, esbuf, len );
	}
	return es;
}

CStringA _tagXMLEntitys::Entity2Ref( LPCSTR str )
{
	CStringA s;
	if( str )
	{
		int nEntityCount = GetEntityCount(str);
		if( nEntityCount == 0 )
			return CStringA(str);
		int len = strlen(str) + nEntityCount * 10;
		LPSTR sbuf = s.GetBufferSetLength( len+1 );
		if( sbuf )
			Entity2Ref( str, sbuf, len );
	}
	return s;
}

CStringA XRef2Entity( LPCSTR estr )
{
	CStringA a;
	return a;
//	return entityDefault.Ref2Entity( estr );
}

CStringA XEntity2Ref( LPCSTR str )
{
	CStringA a;
	return a;
//	return entityDefault.Entity2Ref( str );
}