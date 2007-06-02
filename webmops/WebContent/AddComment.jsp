<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="Add comment">
	<stripes:layout-component name="contents">
		<h2>Add comment to package ${actionBean.pkg.name}.</h2>
		<br />
		<stripes:form beanclass="ru.rpunet.webmops.web.AddCommentActionBean">
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td colspan="2">
						<stripes:errors />
					</td>
				</tr>
				<tr>
					<td><stripes:label for="comment.header" />: </td>
					<td><stripes:text name="comment.header" /></td>
				</tr>
				<tr>
					<td><stripes:label for="comment.message" />: </td>
					<td>&nbsp;</td>
				</tr>
				<tr>
					<td colspan="2">
						<stripes:textarea name="comment.message"> </stripes:textarea>
					</td>
				</tr>
				<tr>
					<td colspan="2">
						<stripes:hidden name="packageId" value="${actionBean.pkg.id}"/>
						<stripes:hidden name="comment.user" value=${user.id}" />
						<stripes:submit name="addComment" value="Add comment" />
					</td>
				</tr>
			</table>
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>