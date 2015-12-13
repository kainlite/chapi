/* Initialization */
int	init(int);

/* Not implemented */
int	serve_index(struct http_request *);
int	serve_apiw1(struct http_request *);

/* Registration */
int	serve_sign_up(struct http_request *);
int	serve_confirm_email(struct http_request *req);
