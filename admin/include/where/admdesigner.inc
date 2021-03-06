<?php

# $Id: admdesigner.inc,v 1.6 2003/01/31 04:13:04 jimmerman Exp $

// Written by James Flemer
// <jflemer@alum.rpi.edu>
// <jflemer@acm.rpi.edu>

	$bg1 =& $ESPCONFIG['bgalt_color1'];
	$bg2 =& $ESPCONFIG['bgalt_color2'];

	$errstr = '';
	$u = '';
	$r = '';

	/* abort */
	if(isset($HTTP_POST_VARS['cancel'])) {
		unset($HTTP_POST_VARS['submit']);
		unset($HTTP_POST_VARS['delete']);
		include(esp_where('designers'));
		return;
	}

	/* delete user */
	if(isset($HTTP_POST_VARS['delete'])) {
		unset($HTTP_POST_VARS['submit']);
		$u = _addslashes($HTTP_POST_VARS['u']);
		$r = _addslashes($HTTP_POST_VARS['r']);
		/* check acl */
		if($xoopsUser->isAdmin() ||
			in_array($r, $HTTP_SESSION_VARS['acl']['pgroup']) ||
			auth_no_access(_('to access this group'))) {
			$sql = "DELETE FROM ".TABLE_DESIGNER." WHERE username='$u' AND realm='$r'";
			if(!mysql_query($sql) || mysql_affected_rows() < 1) {
				/* unsucessfull -- abort */
				$errstr .= mkerror(_('Cannot delete account.') .' ('.
					mysql_error() .')');
			}
		}
		if(empty($errstr)) {
			include(esp_where('designers'));
			return;
		}
	}

	/* submitted */
	if(isset($HTTP_POST_VARS['submit'])) {
		$u         = _addslashes($HTTP_POST_VARS['u']);
		$r         = _addslashes($HTTP_POST_VARS['r']);
		$password  = _addslashes($HTTP_POST_VARS['password']);
		$fname     = _addslashes($HTTP_POST_VARS['fname']);
		$lname     = _addslashes($HTTP_POST_VARS['lname']);
		$email     = _addslashes($HTTP_POST_VARS['email']);
		$pdesign   = _addslashes($HTTP_POST_VARS['pdesign']);
		$pstatus   = _addslashes($HTTP_POST_VARS['pstatus']);
		$pdata     = _addslashes($HTTP_POST_VARS['pdata']);
		$pall      = _addslashes($HTTP_POST_VARS['pall']);
		$pgroup    = _addslashes($HTTP_POST_VARS['pgroup']);
		$puser     = _addslashes($HTTP_POST_VARS['puser']);
		$disabled  = _addslashes($HTTP_POST_VARS['disabled']);

		if(!empty($HTTP_POST_VARS['ex_year']) ||
				!empty($HTTP_POST_VARS['ex_month']) ||
				!empty($HTTP_POST_VARS['ex_day'])) {
			if(empty($HTTP_POST_VARS['ex_day'])) {
				$ex_day = 1;
			} else {
				$ex_day = intval($HTTP_POST_VARS['ex_day']);
			}
			if(empty($HTTP_POST_VARS['ex_month'])) {
				$ex_month = 1;
			} else {
				$ex_month = intval($HTTP_POST_VARS['ex_month']);
			}
			if(empty($HTTP_POST_VARS['ex_year'])) {
				$now = getdate(time());
				$ex_year = $now['year'];
			} else {
				$ex_year = intval($HTTP_POST_VARS['ex_year']);
				if($ex_year < 2000)
					$ex_year += 2000;
			}

			$expiration = sprintf("%04d%02d%02d%06d",
				$ex_year,$ex_month,$ex_day,0);
		} else {
			$expiration = "0";
			$ex_year    = '';
			$ex_month   = '';
			$ex_day     = '';
		}

		if($r == 'superuser' && $HTTP_SESSION_VARS['acl']['superuser'] != 'Y')
			$r = '';

		/* new user */
		$sql = "SELECT * FROM ".TABLE_DESIGNER." WHERE username='$u' AND realm='$r'";
		$result = mysql_query($sql);
		if(!$result || mysql_num_rows($result) < 1) {
			if(empty($u) || empty($r) || empty($password)) {
				$errstr .= mkerror(_('Username, Password, and Group are required.'));
			} else {
				$sql = "INSERT INTO ".TABLE_DESIGNER."
					(username,realm,password)
					VALUES ('$u','$r',PASSWORD('$password'))";
				if(!mysql_query($sql)) {
					$u = '';
					$r = '';
					$errstr .= mkerror(_('Error adding account.') .' ('.
						mysql_error() .')');
				}
			}
		}
		mysql_free_result($result);

		/* change user data */
		if(!empty($u) && !empty($r)) {
			if(empty($pdesign))
				$pdesign = 'Y';
			$fields = array('pstatus','pdata','pall','pgroup','puser','disabled');
			foreach($fields as $f) {
				if(empty($$f))
					$$f = 'N';
			}
			if (!empty($password))
				$password = "password=PASSWORD('$password'),";
			$sql = "UPDATE ".TABLE_DESIGNER." SET
				$password
				fname='$fname',
				lname='$lname',
				email='$email',
				pdesign='$pdesign',
				pstatus='$pstatus',
				pdata='$pdata',
				pall='$pall',
				pgroup='$pgroup',
				puser='$puser',
				disabled='$disabled',
				changed=now(),
				expiration='$expiration'
			WHERE username='$u' AND realm='$r'";
			if(!mysql_query($sql)) {
				/* unsucessfull -- abort */
				$errstr .= mkerror(_('Cannot change account data.') .' ('.
					mysql_error() .')');
			}
		}

		if(empty($errstr)) {
			include(esp_where('designers'));
			return;
		}
	} else if(isset($HTTP_GET_VARS['u']) && isset($HTTP_GET_VARS['r'])) {
		$u = _addslashes($HTTP_GET_VARS['u']);
		$r = _addslashes($HTTP_GET_VARS['r']);
	} else {
		$u = '';
		$r = '';
		$expiration = '';
		$disabled = 'N';
		$pdesign  = 'Y';
		$pstatus  = 'N';
		$pdata    = 'N';
		$pall     = 'N';
		$pgroup   = 'N';
		$puser    = 'N';
	}

	/* load ACL */
	if(!empty($u) && !empty($r)) {
		$sql = "SELECT * FROM ".TABLE_DESIGNER." WHERE username='$u' AND realm='$r'";
		$result = mysql_query($sql);
		if($arr = mysql_fetch_array($result, MYSQL_ASSOC)) {
			foreach(array(
				'username', 'realm', 'fname', 'lname', 'email',
				'pdesign', 'pstatus', 'pdata', 'pall', 'pgroup', 'puser',
				'disabled', 'expiration') as $col)
				{ $$col = $arr[$col]; }
			$u =& $username;
			$r =& $realm;
			if(intval($expiration) > 0) {
				$ex_year  = substr($expiration,0,4);
				$ex_month = substr($expiration,4,2);
				$ex_day   = substr($expiration,6,2);
			} else {
				$ex_year  = '';
				$ex_month = '';
				$ex_day   = '';
			}
		} else {
			$errstr .= mkerror(_('Account not found.') .' ('.
				mysql_error() .')');
		}
	}

?>
<h2><?php mb_echo('Designer Account Administration'); ?></h2>
<?php if(!empty($errstr)) echo("<p>$errstr</p>\n"); ?>
<input type="hidden" name="where" value="admdesigner">
<table border="0" cellspacing="0" cellpadding="4" align="center" bgcolor="<?php echo($ESPCONFIG['active_bgcolor']); ?>">
	<tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Username') ."</th>\n");
	if(empty($u))
		$u = mktext('u', 16, 16, $GLOBALS);
	else
		$u = mkhidden('u', $GLOBALS) . "<tt>$u</tt>";
	echo("<td bgcolor=\"$bg1\">$u</td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Password') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mkpass('password') ."</td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Group') ."</th>\n");
	if(empty($r)) {
		$r = '<select name="r"><option></option>';
		$groups = array();
		if($xoopsUser->isAdmin()) {
			$sql = "SELECT name FROM ".TABLE_REALM."";
			$result = mysql_query($sql);
			while( list($g) = mysql_fetch_row($result) ) {
				array_push($groups, $g);
			}
			mysql_free_result($result);
		} else {
			$groups =& $HTTP_SESSION_VARS['acl']['pgroup'];
		}
		foreach($groups as $g) {
			$r .= "<option value=\"$g\">$g</option>";
		}
		$r .= '</select>';
	} else {
		$r = mkhidden('r', $GLOBALS) . "<tt>$r</tt>";
	}
	echo("<td bgcolor=\"$bg1\"><tt>$r</tt></td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('First Name') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mktext('fname', 16, 16, $GLOBALS) ."</td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Last Name') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mktext('lname', 24, 24, $GLOBALS) ."</td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Email') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mktext('email', 24, 64, $GLOBALS) ."</td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Expiration') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">".
		mktext('ex_year',4,4, $GLOBALS) .' '.
		mktext('ex_month',2,2, $GLOBALS) .' '.
		mktext('ex_day',2,2, $GLOBALS) .' ('.
			_('year') .' '.
			_('month') .' '.
			_('day'). ")</td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Disabled') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mkselect('disabled',array('Y' => mb_('Yes'), 'N' => mb_('No')), $GLOBALS) ."</td>\n");
?>
	</tr><!-- <tr>
		<th colspan="2" bgcolor="<?php echo($bg2); ?>">
			<input type="submit" name="submit" value="<?php mb_echo('Update'); ?>">&nbsp;
			<input type="submit" name="cancel" value="<?php mb_echo('Cancel'); ?>">&nbsp;
			<input type="submit" name="delete" value="<?php mb_echo('Delete'); ?>"></th>
	</tr>--><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Design Surveys') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mkselect('pdesign',array('Y' => mb_('Yes'), 'N' => mb_('No'), $GLOBALS) ."</td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Change Survey Status') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mkselect('pstatus',array('Y' => mb_('Yes'), 'N' => mb_('No')), $GLOBALS) .
		'<!-- ('. mb_('Activate/End') .") --></td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Export Survey Data') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mkselect('pdata',array('Y' => mb_('Yes'), 'N' => mb_('No')), $GLOBALS) ."</td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Group Editor') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mkselect('pall',array('Y' => mb_('Yes'), 'N' => mb_('No')), $GLOBALS) .
		'<!-- ('. mb_('may edit <b>all</b> forms owned by this group') .") --></td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Administer Group Members') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mkselect('pgroup',array('Y' => mb_('Yes'), 'N' => mb_('No')), $GLOBALS) ."</td>\n");
?>
	</tr><tr>
<?php
	echo("<th bgcolor=\"$bg2\" align=\"right\">". mb_('Administer Group Respondents') ."</th>\n");
	echo("<td bgcolor=\"$bg1\">". mkselect('puser',array('Y' => mb_('Yes'), 'N' => mb_('No')), $GLOBALS) ."</td>\n");
?>
	</tr><tr>
		<th colspan="2" bgcolor="<?php echo($bg2); ?>">
			<input type="submit" name="submit" value="<?php mb_echo('Update'); ?>">&nbsp;
			<input type="submit" name="cancel" value="<?php mb_echo('Cancel'); ?>">&nbsp;
			<input type="submit" name="delete" value="<?php mb_echo('Delete'); ?>">
		</th>
	</tr>
</table>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>