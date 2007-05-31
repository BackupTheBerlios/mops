<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="index">
	<stripes:layout-component name="contents">
		<div class="login-box">
		<stripes:form beanclass="ru.rpunet.webmops.web.LoginActionBean" focus="login">
			<stripes:errors />
			<stripes:label for="login" />: <stripes:text name="login" /><br/>
			<stripes:label for="password" />: <stripes:password name="password" /><br/>
			<stripes:label for="remember" />: <stripes:checkbox name="remember" /><br/>
			<stripes:hidden name="targetUrl" value="${request.parameterMap['targetUrl']}" />
			<stripes:submit name="doLogin" value="Login" />
			
		</stripes:form><br />
		<stripes:link beanclass="ru.rpunet.webmops.web.PasswordReminderActionBean">Lost password?</stripes:link> | 
		<stripes:link beanclass="ru.rpunet.webmops.web.RegisterActionBean">Register</stripes:link>
		</div>
	</stripes:layout-component>
</stripes:layout-render>