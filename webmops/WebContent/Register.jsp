<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="Register ">
	<stripes:layout-component name="contents">
	<h2>Registration</h2>
	<br />
	<stripes:form beanclass="ru.rpunet.webmops.web.RegisterActionBean" focus="user.login">
		<table cellpadding="3" cellspacing="0" border="0">
			<tr>
				<td colspan="2">
					<stripes:errors />
				</td>
			</tr>
			<tr>
				<td><stripes:label for="user.login" />: </td>
				<td><stripes:text name="user.login" /></td>
			</tr>
			<tr>
				<td><stripes:label for="user.firstName" />: </td>
				<td><stripes:text name="user.firstName" /></td>
			</tr>
			<tr>
				<td><stripes:label for="user.lastName" />: </td>
				<td><stripes:text name="user.lastName" /></td>
			</tr>
			<tr>
				<td><stripes:label for="user.email" />: </td>
				<td><stripes:text name="user.email" /></td>
			</tr>
			<tr>
				<td><stripes:label for="user.password" />: </td>
				<td><stripes:password name="user.password" /></td>
			</tr>
			<tr>
				<td><stripes:label for="confirmPassword" />: </td>
				<td><stripes:password name="confirmPassword" /></td>
			</tr>
			<tr>
				<td style="text-aling: center;" colspan="2">
					<stripes:submit name="registerUser" value="Register" />
				</td>
			</tr>
		</table>	
	</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>