<?php
// $Id: index.php,v 1.15.1 2004/06/02 12:39:20 dashboard Exp $

require('../../mainfile.php');
require(XOOPS_ROOT_PATH.'/header.php');
include_once('bmefUtils.php');
if($xoopsTpl){
	bmefUtils::assign_message($xoopsTpl);
}
	$xoopsTpl->assign('survey', bmefUtils::get_survey_list());
	$xoopsTpl->assign('version',"0.11" );
	$xoopsOption['template_main'] = 'bmef_list.html';


include(XOOPS_ROOT_PATH.'/footer.php');


?>
