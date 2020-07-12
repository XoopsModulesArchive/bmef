<?php

# $Id: survey_render.inc,v 1.19 2003/03/05 19:26:30 jimmerman Exp $

// Written by James Flemer
// For eGrad2000.com
// <jflemer@alum.rpi.edu>

// Modify by Yoshi Sakai
// For Attach File
// <webmaster@bluemooninc.biz>
// 2004/05/23 Change form style more compact.
// 2004/05/26 Add Attach File.

/* {{{ proto bool survey_render(int survey_id, int section, string message)
   Reads current form variables from HTTP_POST_VARS.
   Builds HTML for the survey (prefixed w/ message). */
function survey_render_smarty($sid, $section = 1, $message = '') {
	global $xoopsTpl,$maxfilesize;
	global $HTTP_POST_VARS;
	@reset($HTTP_POST_VARS);

	if(empty($section)) $section = 1;

	$has_choices = esp_type_has_choices();

	// load survey title (and other globals)
	$sql = "SELECT * FROM ".TABLE_SURVEY." WHERE id='${sid}'";
	$result = mysql_query($sql);
	if(mysql_num_rows($result) != 1) return(false);
	$survey = mysql_fetch_array($result,MYSQL_ASSOC);
	mysql_free_result($result);

	$sections = survey_get_sections($sid);
	$num_sections = count($sections);
	if($section-- > $num_sections) return(false);	// invalid section

    $section_questions = survey_get_section_questions($sid);

	// load survey questions
	$sec_sql = array_to_insql($sections[$section]);
	$sql = "SELECT * FROM ".TABLE_QUESTION." WHERE id $sec_sql ORDER BY position,id";
	$questions_result = mysql_query($sql);
	if(mysql_num_rows($questions_result) < 1) return(false);

	// check to see if there are required questions
	$sql = "SELECT COUNT(*) FROM ".TABLE_QUESTION." WHERE id $sec_sql AND required='Y'";
	$has_required = mysql_result(mysql_query($sql),0,0);

	// find out what question number we are on $i
	$i=0;
	for($j=0;$j<$section;$j++) $i += count($section_questions[$j]);
	$xoopsTpl->assign('survey',$survey);
	$xoopsTpl->assign('message',$message);
	if($num_sections>1) {
		$xoopsTpl->assign('surveyPage',_('Page '.number_format($section+1).' of '.number_format($num_sections) ));
	}
	if($has_required) {
    	$xoopsTpl->assign('has_required',_MD_ASTERISK_REQUIRED);
	}
	$formbody = array();
	$i = 1;
	while($question = mysql_fetch_array($questions_result,MYSQL_ASSOC)) {
		$res = array();
		// process each question
		$qid  = &$question['id'];
		$tid  = &$question['type_id'];
		$size = &$question['length'];
		$prec = &$question['precise'];

		if ($tid == 100) {
			$res['content'] = $question['content'];
			continue;
		}
//		++$i;
		if($has_choices[$tid]) {
			$sql = "SELECT * FROM ".TABLE_QUESTION_CHOICE." WHERE question_id='$qid' AND content NOT LIKE '!other%' ORDER BY id";
			$choices_result = mysql_query($sql);
			$sql = "SELECT * FROM ".TABLE_QUESTION_CHOICE." WHERE question_id='$qid' AND content LIKE '!other%' ORDER BY id";
			$others_result = mysql_query($sql);
			$others = mysql_num_rows($others_result);
		} else {
			$choices_result = ''; 
		}
		$res['number']='';
		if($question['required']=='Y') { 
			$res['number']='<font color="#FF0000">*</font>'; 
		}
		$res['number'] = $res['number'].'<A NAME="Q'.$i.'">'.$i.'</A>';
		$res['content'] = $question['content'];
		$res['body'] ='';
		switch($tid) {
			case '1':	// Yes/No
				$res['body'] = mkradio($qid,'Y')." ".mb_('Yes')
					.mkradio($qid,'N')." ".mb_('No');
				break;
			case '2':	// single text line
				$res['body'] = mktext($qid, $size, $prec);
				break;
			case '3':	// essay
				$res['body'] = mktextarea($qid, $prec, $size, 'VIRTUAL');
				break;
			case '4':	// radio
				while($choice = mysql_fetch_array($choices_result,MYSQL_ASSOC)) {
					$res['body'] = $res['body'].mkradio($qid,$choice['id'])." ".$choice['content'];
				}
				$j=0;
				while($other = mysql_fetch_array($others_result,MYSQL_ASSOC)) {
					$cid = $other['id'];
					$other_text = preg_replace(
							array("/^!other=/","/^!other/"),
							array('','Other:'),
							$other['content']);
					$res['body'] = $res['body'].mkradio($qid,"other_$cid");
                    $cid = "${qid}_${cid}";
                    $res['body'] = $res['body']."$other_text&nbsp;<input type=\"text\" size=\"20\" name=\"$cid\" onKeyPress=\"other_check(this.name)\"";
                	if (isset($GLOBALS['HTTP_POST_VARS'][$cid]))
                        $res['body'] = $res['body'].' value="'. htmlspecialchars($GLOBALS['HTTP_POST_VARS'][$cid]) .'" />';
					$j++;
				}
				break;
			case '5':	// check boxes
				$num_rows = mysql_num_rows($choices_result); 
				$num_alt=0;
				if ($num_rows > 3) $num_alt = ceil( $num_rows / 2 );
				$num=0;
				while($choice = mysql_fetch_array($choices_result,MYSQL_ASSOC)) {	
					if ( ($num_alt>0) && ($num==$num_alt) ){
					}
					$res['body'] = $res['body'].mkcheckbox($qid,$choice['id'])." ".$choice['content'];
					$num++;
				}
				$j=0;
				while($other = mysql_fetch_array($others_result,MYSQL_ASSOC)) {
					$cid = $other['id'];
					$other_text = preg_replace(
							array("/^!other=/","/^!other/"),
							array('','Other:'),
							$other['content']);
							$res['body'] = $res['body'].mkcheckbox($qid,"other_$cid");
                    $cid = "${qid}_${cid}";
                    $res['body'] = $res['body']."$other_text&nbsp;<input type=\"text\" size=\"20\" name=\"$cid\" onKeyPress=\"other_check(this.name)\"";
                	if (isset($GLOBALS['HTTP_POST_VARS'][$cid]))
                        $res['body'] = $res['body'].' value="'. htmlspecialchars($GLOBALS['HTTP_POST_VARS'][$cid]) .'" />';
					$j++;
				}
				break;
			case '6':	// dropdown box
				$options = array();
				while($choice = mysql_fetch_array($choices_result,MYSQL_ASSOC)) {
					$options[$choice['id']] = $choice['content'];
				}
				$res['body'] = mkselect($qid,$options);
				break;
			case '7':	// rating
				$res['body'] = '<table border="0" cellspacing="0" cellpadding="0"><tr>';
				$res['body'] = $res['body'].'<td width="60">'.mkradio($qid,1)." ".mb_('1').'</td>';
				$res['body'] = $res['body'].'<td width="60">'.mkradio($qid,2)." ".mb_('2').'</td>';
				$res['body'] = $res['body'].'<td width="60">'.mkradio($qid,3)." ".mb_('3').'</td>';
				$res['body'] = $res['body'].'<td width="60">'.mkradio($qid,4)." ".mb_('4').'</td>';
				$res['body'] = $res['body'].'<td width="60">'.mkradio($qid,5)." ".mb_('5').'</td>';
				$res['body'] = $res['body'].'<td width="60">'.mkradio($qid,'N/A')." ".mb_('N/A').'</td></tr></table>';
				break;
			case '8':	// ranking
				$bg = '#eeeeee';	// bgcolor
				$res['body'] = '<table border="0" cellspacing="1" cellpadding="0"><tr><td width="20%"></td>';
				for ($j = 0; $j < $size; $j++) {
					$res['body'] = $res['body'].
						'<td width="16%" align="center" bgcolor="'.$bg.'">'.number_format($j+1).'</td>';
					if ($bg == '#eeeeee')   $bg = '#dddddd';
					else                    $bg = '#eeeeee';
				}
				if ($prec) {
					$res['body'] = $res['body'].
						'<td width="20%" align="center" bgcolor="'.$bg.'">'.mb_('N/A').'</td>';
				}
				$res['body'] = $res['body'].'</tr>';
				while($choice = mysql_fetch_array($choices_result,MYSQL_ASSOC)) {
					$cid = $choice['id'];
					$str = "${qid}_$cid";
					
					$res['body'] = $res['body'].'<tr><td width="20%" align="right">'.$choice['content'].'</td>';
					$bg = '#eeeeee';
					for ($j = 0; $j < $size; $j++) {
						$res['body'] = $res['body'].
							'<td width="16%" align="center" bgcolor="'.$bg.'">'.mkradio($str,$j).'</td>';
						if ($bg == '#eeeeee')   $bg = '#dddddd';
						else                    $bg = '#eeeeee';
					}
					if ($prec) {
						$res['body'] = $res['body'].
							'<td width="20%" align="center" bgcolor="'.$bg.'">'.mkradio($str,'N/A').'</td>';
					}
					$res['body'] = $res['body']."</tr>";
				}
				$res['body'] = $res['body']."</table>";
				break;
			case '9':	// date
				$res['body'] = mktext($qid, 10, 10).'<em>(e.g. 2004/7/19)</em>';
				break;
			case '10':	// numeric
				$size++; // for sign
				if($prec) $size += 1 + $prec;
				$res['body'] = mktext($qid, $size, $size);
				break;
			case '40':	// Attache
				$maxbyte=$maxfilesize;
				$res['body'] = $res['body']."<INPUT TYPE='hidden' NAME='MAX_FILE_SIZE' VALUE='2000000'>".
					"<INPUT type='file' size='".$size."' name='".$qid."' /> Max 2MByte.";
				break;
		}
		// end of select
		$formbody[$i] = $res;
		$i++;
	}
	// end of questions
	if($num_sections>1) {
		$res['body'] = $res['body'].'<font size="-1" class="surveyPage">'.mb_('Page')
			.number_format($section+1).mb_('of').number_format($num_sections);
	}
	return $formbody;
}
/*
** For Preview
*/
function survey_render($sid, $section = 1, $message = '') {
	global $HTTP_POST_VARS;
	@reset($HTTP_POST_VARS);

	if(empty($section))
		$section = 1;

	$has_choices = esp_type_has_choices();

// load survey title (and other globals)
	$sql = "SELECT * FROM ".TABLE_SURVEY." WHERE id='${sid}'";
	$result = mysql_query($sql);
	if(mysql_num_rows($result) != 1)
		return(false);
	$survey = mysql_fetch_array($result,MYSQL_ASSOC);
	mysql_free_result($result);

	$sections = survey_get_sections($sid);
	$num_sections = count($sections);
	if($section-- > $num_sections)
		return(false);	// invalid section

    $section_questions = survey_get_section_questions($sid);

// load survey questions
	$sec_sql = array_to_insql($sections[$section]);
	$sql = "SELECT * FROM ".TABLE_QUESTION." WHERE id $sec_sql ORDER BY position,id";
	$questions_result = mysql_query($sql);
	if(mysql_num_rows($questions_result) < 1)
		return(false);

// check to see if there are required questions
	$sql = "SELECT COUNT(*) FROM ".TABLE_QUESTION." WHERE id $sec_sql AND required='Y'";
	$has_required = mysql_result(mysql_query($sql),0,0);

// find out what question number we are on $i
	$i=0;
	for($j=0;$j<$section;$j++)
		$i += count($section_questions[$j]);
?>
<table class="headerGraphic">
<tr>
	<td class="image"></td>
</tr><tr>
<!--<h2 class="surveyTitle"><?php echo($survey["title"]); ?></h2>-->
<!--<h3 class="surveySubtitle"><?php echo($survey["subtitle"]); ?></h3>-->
<td class="surveyTitle">
<?php echo($survey["title"]); ?>
<?php echo($survey["subtitle"]); ?>
<?php if($num_sections>1) { ?>
	<font size="-1" class="surveyPage"><?php printf(_('Page %d of %d'), $section+1, $num_sections); ?></font>
<?php } ?>
</td></tr><tr>
<td class="addInfo"><?php echo($survey["info"]); ?></td></tr><tr>
<td class="message"><?php echo($message); ?></td></tr><tr>
<?php if($has_required) { ?>
    <td class="reqQuestion"><font size="-1">
    <?php printf(_('Questions marked with a %s are required.'), '<font color="#FF0000">*</font>'); ?>
    </font></td>
<?php } ?>
</tr>
</table>
<!--<blockquote class="addInfo"><?php echo($survey["info"]); ?></blockquote>
<blockquote class="message"><?php echo($message); ?></blockquote>-->
<div align="center">
<table class="mainTable">
<?php
	while($question = mysql_fetch_array($questions_result,MYSQL_ASSOC)) {
		// process each question
		$qid  = &$question['id'];
		$tid  = &$question['type_id'];
		$size = &$question['length'];
		$prec = &$question['precise'];

		if ($tid == 100) {

?>
<!--<tr>
    <td class="preQuestionBorder">&nbsp;</td>
</tr>-->
<tr>
    <td height="18" vAlign="top">
        <table class="qnOuterTable" width="100%" border="0" cellpadding="1" cellspacing="1">
        <tr>
            <td width="100%" height="18" valign="top">
                <table class="qnInnerTable" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr>
        	        <td class="addInfo" width="100%" height="18">&nbsp;
        	            <?php echo $question['content']; ?>
        	        </td>
                    </tr>
                </table>
            </td>
        </tr>
        </table>
    </td>
</tr>
<?php

			continue;
		}

		++$i;
		if($has_choices[$tid]) {
			$sql = "SELECT * FROM ".TABLE_QUESTION_CHOICE." WHERE question_id='$qid' AND content NOT LIKE '!other%' ORDER BY id";
			$choices_result = mysql_query($sql);
			$sql = "SELECT * FROM ".TABLE_QUESTION_CHOICE." WHERE question_id='$qid' AND content LIKE '!other%' ORDER BY id";
			$others_result = mysql_query($sql);
			$others = mysql_num_rows($others_result);
		} else { $choices_result = ''; }
?>
<tbody>
    <!--
      <tr>
        <td class="preQuestionBorder" >&nbsp;</td>
      </tr>
    -->
      <tr>
        <td height="0" vAlign=middle>
        <table class="qnOuterTable" width="100%" cellspacing="0">
            <tr><td width="100%" height="10" valign="top">
                 <table class="qnInnerTable" width="100%" cellPadding="0"  cellSpacing="1">
                 <tr>
                 <td class="qnInnerTd" rowspan="0" vAlign="top">
			<?php
			if($question['required']=='Y') { 
				echo('<font color="#FF0000">*</font>'); 
			}
			?>
			<A NAME="Q<?php echo($i); ?>"><?php echo($i); ?>.</A></td>
			<?php
			    ?><td class="qnInner">
				<?php echo($question['content']); ?></td>
				<!-- </tr><tr> -->
				<td class="qnType">
				<!--<blockquote>-->
<?php
		switch($tid) {
			case '1':	// Yes/No
?>
				<table cellSpacing="0" cellPadding="0">
                        	<tbody>
                          	<tr>
                            		<td><?php echo(mkradio($qid,'Y')); ?></td>
                            		<td><?php mb_echo('Yes'); ?></td>
                          	</tr>
                          	<tr>
                            		<td><?php echo(mkradio($qid,'N')); ?></td>
                            		<td><?php mb_echo('No'); ?></td>
                          	</tr>
                        	</tbody>
                      		</table>
<?php
				break;
			case '2':	// single text line
?>
					<?php echo(mktext($qid, $size, $prec)); ?>
<?php
				break;
			case '3':	// essay
?>
					<?php echo(mktextarea($qid, $prec, $size, 'VIRTUAL')); ?>
<?php
				break;
			case '4':	// radio
?>
					<table border="0" cellspacing="0" cellpadding="0">
					<tbody>
<?php			while($choice = mysql_fetch_array($choices_result,MYSQL_ASSOC)) {	?>
						<!--<tr>-->
							<td><?php echo(mkradio($qid,$choice['id'])); ?></td>
							<td class="qnType"><?php echo($choice['content']); ?>&nbsp;</td>
						<!--</tr>-->
<?php			}
				$j=0;
				while($other = mysql_fetch_array($others_result,MYSQL_ASSOC)) {
					$cid = $other['id'];
					$other_text = preg_replace(
							array("/^!other=/","/^!other/"),
							array('','Other:'),
							$other['content']);
?>
						<tr>
							<td><?php echo(mkradio($qid,"other_$cid")); ?></td>
							<td class="qnType">
<?php
                    $cid = "${qid}_${cid}";
                    echo("$other_text&nbsp;<input type=\"text\" size=\"20\" name=\"$cid\" onKeyPress=\"other_check(this.name)\"");
                	if (isset($GLOBALS['HTTP_POST_VARS'][$cid]))
                        echo(' value="'. htmlspecialchars($GLOBALS['HTTP_POST_VARS'][$cid]) .'"');
                    echo(" />");
?>
                            </td>
						</tr>
<?php
					$j++;
				}
?>
					</tbody>
					</table>
<?php
				break;
			case '5':	// check boxes
?>
					<table border="0" cellspacing="0" cellpadding="0">
					<tbody><tr>
<?php			$num_rows = mysql_num_rows($choices_result); 
				$num_alt=0;
				if ($num_rows > 3) $num_alt = ceil( $num_rows / 2 );
				$num=0;
				while($choice = mysql_fetch_array($choices_result,MYSQL_ASSOC)) {	
					if ( ($num_alt>0) && ($num==$num_alt) ){
						?></tr><tr><?php
					}
						?><!--<tr>-->
							<td><?php echo(mkcheckbox($qid,$choice['id'])); ?></td>
							<td class="qnType"><?php echo($choice['content']); ?>&nbsp;</td>
						<!--</tr>--><?php
					$num++;
				}
				$j=0;
				while($other = mysql_fetch_array($others_result,MYSQL_ASSOC)) {
					$cid = $other['id'];
					$other_text = preg_replace(
							array("/^!other=/","/^!other/"),
							array('','Other:'),
							$other['content']);
?>
						<!--<tr>-->
							<td><?php echo(mkcheckbox($qid,"other_$cid")); ?></td>
							<td class="qnType">
<?php
                    $cid = "${qid}_${cid}";
                    echo("$other_text&nbsp;<input type=\"text\" size=\"20\" name=\"$cid\" onKeyPress=\"other_check(this.name)\"");
                	if (isset($GLOBALS['HTTP_POST_VARS'][$cid]))
                        echo(' value="'. htmlspecialchars($GLOBALS['HTTP_POST_VARS'][$cid]) .'"');
                    echo(" />");
?>
                            </td>

						<!--</tr>-->
<?php
					$j++;
				}
?>
					</tr>
					</tbody>
					</table>
<?php
				break;
			case '6':	// dropdown box
				$options = array();
?>
<?php				while($choice = mysql_fetch_array($choices_result,MYSQL_ASSOC)) {
					$options[$choice['id']] = $choice['content'];
				}
?>
					<?php echo(mkselect($qid,$options)); ?>
<?php
				break;
			case '7':	// rating
?>
					<table border="0" cellspacing="0" cellpadding="0">
					<tbody>
						<tr>
							<td width="60"><?php echo(mkradio($qid,1)); ?> <?php mb_echo('1'); ?></td>
							<td width="60"><?php echo(mkradio($qid,2)); ?> <?php mb_echo('2'); ?></td>
							<td width="60"><?php echo(mkradio($qid,3)); ?> <?php mb_echo('3'); ?></td>
							<td width="60"><?php echo(mkradio($qid,4)); ?> <?php mb_echo('4'); ?></td>
							<td width="60"><?php echo(mkradio($qid,5)); ?> <?php mb_echo('5'); ?></td>
							<td width="60"><?php echo(mkradio($qid,'N/A')); ?> <?php mb_echo('N/A'); ?></td>
						</tr>
					</tbody>
					</table>
<?php
				break;
			case '8':	// ranking
?>
					<table border="0" cellspacing="1" cellpadding="0">
						<tbody>
						<tr>
							<td></td>
<?php
				$bg = '#eeeeee';
				for ($j = 0; $j < $size; $j++) {
?>
							<td width="40" align="center" bgcolor="<?php echo($bg); ?>"><?php echo($j+1); ?></td>
<?php
					if ($bg == '#eeeeee')   $bg = '#dddddd';
					else                    $bg = '#eeeeee';
				}
				if ($prec) {
?>
							<td width="40" align="center" bgcolor="<?php echo($bg); ?>"><?php mb_echo('N/A'); ?></td>
<?php
				}
?>
						</tr>
<?php
				while($choice = mysql_fetch_array($choices_result,MYSQL_ASSOC)) {
					$cid = $choice['id'];
					$str = "${qid}_$cid";
?>
						<tr>
							<td align="right"><?php echo($choice['content']); ?></td>
<?php
					$bg = '#eeeeee';
					for ($j = 0; $j < $size; $j++) {
?>
							<td width="40" align="center" bgcolor="<?php echo($bg); ?>"><?php echo(mkradio($str,$j)); ?></td>
<?php
						if ($bg == '#eeeeee')   $bg = '#dddddd';
						else                    $bg = '#eeeeee';
					}
					if ($prec) {
?>
							<td width="40" align="center" bgcolor="<?php echo($bg); ?>"><?php echo(mkradio($str,'N/A')); ?></td>
<?php
					}
?>
						</tr>
<?php			} ?>
					</tbody>
					</table>
<?php
				break;
			case '9':	// date
?>
					<?php echo(mktext($qid, 10, 10)); ?> <em>(e.g. 2004/7/19)</em>
<?php
				break;
			case '10':	// numeric
					$size++; // for sign
					if($prec)
						$size += 1 + $prec;
?>
					<?php echo(mktext($qid, $size, $size)); ?>
<?php
				break;
			case '40':	// Attache
				//echo(mktext($qid, $size, $prec));
				$maxbyte = upload_max_filesize;
				echo "<INPUT TYPE='hidden' NAME='MAX_FILE_SIZE' VALUE='2000000'>";
				echo "<INPUT type='file' size='".$size."' name='".$qid."' /> Max 2MByte.";
				break;
		}
		// end of select
?>
		<!--</blockquote>-->
                         </td>
                  </tr>
                </table></td>
            </tr>
          </table>
          </td>
      </tr></tbody>
<?php
	}
	// end of questions
?>
</table>
</div>
<?php if($num_sections>1) { ?>
	<font size="-1" class="surveyPage"><?php mb_echo('Page'); ?>
	<?php echo($section+1); ?>
	<?php mb_echo('of'); ?>
	<?php echo($num_sections); ?></font><br>
<?php } ?>
<?php
	return;
}

?>
