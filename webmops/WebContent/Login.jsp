<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="index">
	<stripes:layout-component name="contents">
		<h2>Login</h2><br/>
		<div class="login-box">
		<stripes:form beanclass="ru.rpunet.webmops.web.LoginActionBean" focus="login">
		<table width="60%" cellpadding="3" cellspacing="0" border=0">
			<tr>
				<td colspan="2">
					<stripes:errors />
				</td>
			</tr>
			<tr>
				<td><stripes:label for="login" />: </td>
				<td><stripes:text name="login" /></td>
			</tr>
			<tr>
				<td><stripes:label for="password" />: </td>
				<td><stripes:password name="password" /></td>
			</tr>
			<tr>
				<td><stripes:label for="remember" />: </td>
				<td><stripes:checkbox name="remember" /></td>
			</tr>
			<tr>
				<td colspan=2" style="text-align: center;">
					<stripes:hidden name="targetUrl" value="${request.parameterMap['targetUrl']}" />
					<stripes:submit name="doLogin" value="Login" />
				</td>
			</tr>		
		</table>	
		</stripes:form><br />
		<stripes:link beanclass="ru.rpunet.webmops.web.PasswordReminderActionBean">Lost password?</stripes:link> | 
		<stripes:link beanclass="ru.rpunet.webmops.web.RegisterActionBean">Register</stripes:link>
		</div>
	</stripes:layout-component>
</stripes:layout-render>