/*
**  mod_harbour.c -- Apache sample harbour module
**  [Autogenerated via ``apxs -n harbour -g'']
**
**  To play with this sample module first compile it into a
**  DSO file and install it into Apache's modules directory
**  by running:
**
**    $ apxs -c -i mod_harbour.c
**
**  Then activate it in Apache's apache2.conf file for instance
**  for the URL /harbour in as follows:
**
**    #   apache2.conf
**    LoadModule harbour_module modules/mod_harbour.so
**    <Location /harbour>
**    SetHandler harbour
**    </Location>
**
**  Then after restarting Apache via
**
**    $ apachectl restart
**
**  you immediately can request the URL /harbour and watch for the
**  output of this module. This can be achieved for instance via:
**
**    $ lynx -mime_header http://localhost/harbour
**
**  The output should be similar to the following one:
**
**    HTTP/1.1 200 OK
**    Date: Tue, 31 Mar 1998 14:42:22 GMT
**    Server: Apache/1.3.4 (Unix)
**    Connection: close
**    Content-Type: text/html
**
**    The sample page from mod_harbour.c
*/

#include <stdio.h>
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_core.h"
#include "http_log.h"
#include "http_request.h"
#include "ap_config.h"
#include "ap_provider.h"
#include "apr_hash.h"
#include <dlfcn.h>

typedef struct
{
    char context[256];
    char path[256];
    char libName[256];
} harbour_config;

static harbour_config config;
const char *harbour_set_path(cmd_parms *cmd, void *cfg, const char *arg);
const char *harbour_set_libName(cmd_parms *cmd, void *cfg, const char *arg);
void *create_dir_conf(apr_pool_t *pool, char *context);
void *merge_dir_conf(apr_pool_t *pool, void *BASE, void *ADD);
static void harbour_register_hooks( apr_pool_t * pool );
static int harbour_handler( request_rec * r );


static request_rec * _r;

int ap_headers_in_count( void )
{
   return apr_table_elts( _r->headers_in )->nelts;
}   

char * ap_headers_in_key( int iKey )
{
   const apr_array_header_t * fields = apr_table_elts( _r->headers_in );
   apr_table_entry_t * e = ( apr_table_entry_t * ) fields->elts;
   
   if( iKey >= 0 && iKey < fields->nelts )
      return e[ iKey ].key;
   else
      return "";
}   

char * ap_headers_in_val( int iKey )
{
   const apr_array_header_t * fields = apr_table_elts( _r->headers_in );
   apr_table_entry_t * e = ( apr_table_entry_t * ) fields->elts;
   
   if( iKey >= 0 && iKey < fields->nelts )
      return e[ iKey ].val;
   else
      return "";
}   


static const command_rec directives[] =
{
    AP_INIT_TAKE1("HarbourPath", harbour_set_path, NULL, OR_ALL, "The path to harbour server"),
    AP_INIT_TAKE1("HarbourLib", harbour_set_libName, NULL, OR_ALL, "Library name for Harbour"),
    { NULL }
};

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA harbour_module = {
    STANDARD20_MODULE_STUFF,
    create_dir_conf,        /* create per-dir    config structures */
    merge_dir_conf,         /* merge  per-dir    config structures */
    NULL,                   /* create per-server config structures */
    NULL,                   /* merge  per-server config structures */
    directives,             /* table of config file commands       */
    harbour_register_hooks  /* register hooks                      */
};

static void harbour_register_hooks( apr_pool_t * pool )
{
   ap_hook_handler( harbour_handler, NULL, NULL, APR_HOOK_MIDDLE );
}

static int harbour_handler( request_rec * r )
{
   void * lib_harbour = NULL;
   int ( * _hb_apache )( void * pRequestRec, void * pAPRPuts, 
                         const char * szFileName, const char * szArgs, const char * szMethod, const char * szUserIP,
                         void * pHeadersIn, void * pHeadersOut, void * pHeadersInCount, void * pHeadersInKey,
                         void * pHeadersInVal ) = NULL;
   int iResult = OK;
   char libPath[512];

   if( strcmp( r->handler, "harbour" ) )
      return DECLINED;

   harbour_config *config = (harbour_config *) ap_get_module_config(r->per_dir_config, &harbour_module);

   r->content_type = "text/html";
   _r = r;

   strcpy(libPath, config->path);
   strcat(libPath, config->libName);

   lib_harbour = dlopen( libPath, RTLD_LAZY );

   if( lib_harbour == NULL )
      ap_rputs( dlerror(), r );
   else
   {
      _hb_apache = dlsym( lib_harbour, "hb_apache" );

      if( _hb_apache == NULL )
         ap_rputs( "failed to load hb_apache()", r );
      else
         iResult = _hb_apache( r, ap_rputs, r->filename, r->args, r->method, r->useragent_ip, r->headers_in, r->headers_out, ap_headers_in_count, ap_headers_in_key, ap_headers_in_val );
   }

   if( lib_harbour != NULL )
      dlclose( lib_harbour );

   return iResult;
}

const char *harbour_set_path(cmd_parms *cmd, void *cfg, const char *arg)
{
   harbour_config    *conf = (harbour_config *) cfg;

   if(conf)
      strcpy(conf->path, arg);

   return NULL;
}

const char *harbour_set_libName(cmd_parms *cmd, void *cfg, const char *arg)
{
   harbour_config    *conf = (harbour_config *) cfg;

   if(conf)
      strcpy(conf->libName, arg);

   return NULL;
}

void *create_dir_conf(apr_pool_t *pool, char *context)
{
   context = context ? context : "Newly created configuration";

   harbour_config *cfg = apr_pcalloc(pool, sizeof(harbour_config));

   if(cfg)
   {
      /* Set some default values */
      strcpy(cfg->context, context);
      memset(cfg->path, 0, 256); // strcpy(cfg->path, "/usr/local/apache/htdocs/");
      memset(cfg->libName, 0, 256); // strcpy(cfg->libName, "libharbour.3.2.0.dylib");
   }

   return cfg;
}

void *merge_dir_conf(apr_pool_t *pool, void *BASE, void *ADD)
{
   harbour_config *base = (harbour_config *) BASE;
   harbour_config *add = (harbour_config *) ADD;
   harbour_config *conf = (harbour_config *) create_dir_conf(pool, "Merged configuration");

   strcpy(conf->path, strlen(add->path) ? add->path : base->path);
   strcpy(conf->libName, strlen(add->libName) ? add->libName : base->libName);
   return conf;
}