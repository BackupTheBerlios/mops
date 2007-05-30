<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="index">
	<stripes:layout-component name="contents">
		<stripes:form beanclass="ru.rpunet.webmops.web.LoginActionBean">
			<stripes:errors />
			Login: <stripes:text name="login" /><br/>
			Password: <stripes:password name="password" /><br/>
			<stripes:submit name="doLogin" value="Login" />
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>