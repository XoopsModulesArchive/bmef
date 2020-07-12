<?php
// Max upload file size
// If you want set upper 2M, You must change php.ini
//   memory_limit = 8M (default)
//   post_max_size = 8M (default)
//   upload_max_filesize =2M (default)
$maxfilesize = 2000000;			// default 2MB

if(
	!defined('XOOPS_ROOT_PATH') ||
	!defined('XOOPS_CACHE_PATH') ||
	!is_dir(XOOPS_CACHE_PATH)
){
	exit();
}
if(!empty($xoopsConfig)){
	if(file_exists(XOOPS_ROOT_PATH.'/modules/bmef/language/'.$xoopsConfig['language'].'/main.php')){
		require_once XOOPS_ROOT_PATH.'/modules/bmef/language/'.$xoopsConfig['language'].'/main.php';
	}
}
bmef_init();
/**
 * Here are all the configuration options.
 */
// Base URL for phpESP
// The string $server['HTTP_HOST'] will be replaced by the server name
$ESPCONFIG['base_url'] = XOOPS_URL.'/modules/bmef/';

// URL of the images directory (for <img src='...'> tags)
$ESPCONFIG['image_url'] = $ESPCONFIG['base_url'] . 'images/';

// URL of the automatic survey publisher
$ESPCONFIG['autopub_url'] = $ESPCONFIG['base_url'] . 'public/survey.php';

// URL of the CSS directory (for themes)
$ESPCONFIG['css_url'] = $ESPCONFIG['base_url'] . 'public/css/';

function bmef_init(){
	global $xoopsDB;
	define('TABLE_REALM', $xoopsDB->prefix("bmef_realm"));
	define('TABLE_RESPONDENT', $xoopsDB->prefix("bmef_respondent"));
	define('TABLE_DESIGNER', $xoopsDB->prefix("bmef_designer"));
	define('TABLE_SURVEY', $xoopsDB->prefix("bmef_survey" ));
	define('TABLE_QUESTION_TYPE', $xoopsDB->prefix("bmef_question_type" ));
	define('TABLE_QUESTION', $xoopsDB->prefix("bmef_question" ));
	define('TABLE_QUESTION_CHOICE', $xoopsDB->prefix("bmef_question_choice" ));
	define('TABLE_ACCESS', $xoopsDB->prefix("bmef_access" ));
	define('TABLE_RESPONSE', $xoopsDB->prefix("bmef_response" ));
	define('TABLE_RESPONSE_BOOL', $xoopsDB->prefix("bmef_response_bool" ));
	define('TABLE_RESPONSE_SINGLE', $xoopsDB->prefix("bmef_response_single" ));
	define('TABLE_RESPONSE_MULTIPLE', $xoopsDB->prefix("bmef_response_multiple" ));
	define('TABLE_RESPONSE_RANK', $xoopsDB->prefix("bmef_response_rank" ));
	define('TABLE_RESPONSE_TEXT', $xoopsDB->prefix("bmef_response_text" ));
	define('TABLE_RESPONSE_OTHER', $xoopsDB->prefix("bmef_response_other" ));
	define('TABLE_RESPONSE_DATE', $xoopsDB->prefix("bmef_response_date" ));

	define('TABLE_', $xoopsDB->prefix("bmef_" ));

}
?>
