<?php
// $Id: index.php,v 1.1 2004/01/29 15:28:30 buennagel Exp $
//  ------------------------------------------------------------------------ //
//                XOOPS - PHP Content Management System                      //
//                    Copyright (c) 2000 XOOPS.org                           //
//                       <http://www.xoops.org/>                             //
//  ------------------------------------------------------------------------ //
//  This program is free software; you can redistribute it and/or modify     //
//  it under the terms of the GNU General Public License as published by     //
//  the Free Software Foundation; either version 2 of the License, or        //
//  (at your option) any later version.                                      //
//                                                                           //
//  You may not change or alter any portion of this comment or credits       //
//  of supporting developers from this source code or any supporting         //
//  source code which is considered copyrighted (c) material of the          //
//  original comment or credit authors.                                      //
//                                                                           //
//  This program is distributed in the hope that it will be useful,          //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of           //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
//  GNU General Public License for more details.                             //
//                                                                           //
//  You should have received a copy of the GNU General Public License        //
//  along with this program; if not, write to the Free Software              //
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA //
//  ------------------------------------------------------------------------ //
// Author: Yoshi.S                                                           //
// URL: http://www.bluemooninc.biz/                                          //
// ------------------------------------------------------------------------- //
include '../../../include/cp_header.php';
require_once('phpESP.ini.php');


$espUrl['root'] = XOOPS_URL."/modules/bmef/";
$espUrl['admin'] = $espUrl['root']."admin";
$adminmenu = $espUrl['admin'] . '/manage.php';


print($ESPCONFIG['charset']);

xoops_cp_header();
echo"<table width='100%' border='0' cellspacing='1' class='outer'>"
."<tr><td class=\"odd\">";
echo "<a href='./index.php'><h2>";
mb_echo('Management Interface');
echo "</h2></a>";
if(isset($mode)) {
}else {
?>
	<table border="0" cellpadding="4" cellspacing="1" width="100%"><tr><td>
	<li><a href="<?php echo("$adminmenu?where=new");     ?>"><?php mb_echo('Create a New Survey'); ?></a></li>
	<li><a href="<?php echo("$adminmenu?where=edit");    ?>"><?php mb_echo('Edit an Existing Survey'); ?></a></li>
	<li><a href="<?php echo("$adminmenu?where=test");    ?>"><?php mb_echo('Test a Survey'); ?></a></li>
	<li><a href="<?php echo("$adminmenu?where=copy");    ?>"><?php mb_echo('Copy an Existing Survey'); ?></a></li>
	<li><a href="<?php echo("$adminmenu?where=status");  ?>"><?php mb_echo('Change the Status of a Survey'); ?></a>
	<li><a href="<?php echo("$adminmenu?where=access");  ?>"><?php mb_echo('Change Access To a Survey'); ?></a>
	 (<?php mb_echo('Limit Respondents.'); ?>)</li>
	<li><a href="<?php echo("$adminmenu?where=results"); ?>"><?php mb_echo('View Results from a Survey'); ?></a></li>
	<li><a href="<?php echo("$adminmenu?where=results&type=cross"); ?>"><?php mb_echo('Cross Tabulate Survey Results'); ?></a></li>
	<li><a href="<?php echo("$adminmenu?where=report");  ?>"><?php mb_echo('View a Survey Report'); ?></a></li>
	<li><a href="<?php echo("$adminmenu?where=export");  ?>"><?php mb_echo('Export Data to CSV'); ?></a></li>
	<li><a href="<?php echo("$adminmenu?where=guide");  ?>"><?php mb_echo('View the User &amp; Administrator Guide'); ?></a></li>
	</td></tr></table>
<?php
}
echo "</td></tr></table>";
xoops_cp_footer();
?>